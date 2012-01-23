#include "context.hpp"
#include "log.hpp"

namespace lector
{

    namespace
    {
        void boost_mutex_lock(void* user)
        {
            boost::mutex& mutex = *reinterpret_cast<boost::mutex*>(user);

            mutex.lock();
        }

        void boost_mutex_unlock(void* user)
        {
            boost::mutex& mutex = *reinterpret_cast<boost::mutex*>(user);

            mutex.unlock();
        }
    }

    struct context::shared_data
    {
        shared_data()
            : outline(0), lists(0), xref(0), page_count(0), alloc(0)
        {
        }

        void recycle(fz_context* ctx)
        {
            if (outline)
                fz_free_outline(outline);

            if (lists)
            {
                for (unsigned i = 0; i < page_count; ++i)
                {
                    if (lists[i])
                        fz_free_display_list(ctx, lists[i]);
                }
                delete lists;
            }

            if (xref)
                pdf_free_xref(xref);
        }

        ~shared_data()
        {
            // TODO: Recycle, thus we need to have a context here.
            // delete alloc;
        }

        fz_outline* outline;
        fz_display_list** lists;
        pdf_xref* xref;
        std::size_t page_count;

        fz_alloc_context* alloc;
    };
    
    context::context(std::size_t limit)
        : data_(new shared_data), fitz_mutex_(new boost::mutex),
          data_mutex_(new boost::mutex)
    {
        LECTOR_LOG_FUNC;
        boost::mutex::scoped_lock lock(*data_mutex_);

        data_->alloc = new fz_alloc_context;
        data_->alloc->malloc = fz_alloc_default.malloc;
        data_->alloc->realloc = fz_alloc_default.realloc;
        data_->alloc->free = fz_alloc_default.free;
        data_->alloc->lock = boost_mutex_lock;
        data_->alloc->unlock = boost_mutex_unlock;
        LECTOR_LOG("Initialized data");

        data_->alloc->user = reinterpret_cast<void*>(fitz_mutex_.get());
        data_->lists = 0;

        ctx_ = fz_new_context(data_->alloc, limit << 20);
    }

    context::context(context const& other)
        : fitz_mutex_(other.fitz_mutex_),
          data_mutex_(other.data_mutex_)
    {
        {
            boost::mutex::scoped_lock lock(*data_mutex_);
            data_ = other.data_;
        }

        ctx_ = fz_clone_context(const_cast<fz_context*>(other.ctx_));
    }
   
    context::~context()
    {
        boost::mutex::scoped_lock lock(*data_mutex_);
        data_.reset();
        lock.unlock();

        fz_free_context(ctx_);
    }

    std::size_t context::get_page_count() const
    {
        return data_->page_count;
    }

    // TODO: Move this into the constructor
    void context::load_file(std::string const& name)
    {
        LECTOR_LOG_FUNC;

        pdf_xref* xref = 0;
        std::size_t page_count = 0;
        fz_display_list** lists = 0;

        fz_try(ctx_)
        {
            xref = pdf_open_xref(ctx_, const_cast<char*> (name.c_str()));
            page_count = pdf_count_pages(xref);
            lists = new fz_display_list*[page_count];
            for (unsigned i = 0; i < page_count; ++i)
                lists[i] = 0;
        }
        fz_catch(ctx_)
        {
            xref = 0;
            throw pdf_exception("Couldn't open PDF file");
        }

        {
            boost::mutex::scoped_lock lock(*data_mutex_);

            data_->recycle(ctx_);
            data_->xref = xref;
            data_->page_count = page_count;
            data_->lists = lists;
        }
    }

    fz_display_list* context::get_display_list(std::size_t i)
    {
        pdf_xref* xref = data_->xref;
        std::size_t page_count = data_->page_count;
        if (!xref || page_count < i)
            throw pdf_exception("ERROR");

        fz_display_list* list = data_->lists[i];

        if (!list)
        {
            LECTOR_LOG("Loading list %d of %d", i, page_count);
            list = call_ret<fz_display_list*> (fz_new_display_list);

            fz_context* main_context = xref->ctx;

            {
                // Lock here, because xrefs holds a file descriptor
                boost::mutex::scoped_lock lock(*data_mutex_);

                xref->ctx = ctx_;

                pdf_page* page = 0;
                fz_try(ctx_)
                {
                    page = pdf_load_page(xref, i);
                }
                fz_catch(ctx_)
                {
                    LECTOR_LOG_ERROR("Couldn't load page %d", i);
                    throw pdf_exception("Couldn't load page");
                }

                fz_device* dev = call_ret<fz_device*> (fz_new_list_device, list);

                fz_try(ctx_)
                {
                    pdf_run_page(xref, page, dev, fz_identity, 0);
                }
                fz_catch(ctx_)
                {
                    LECTOR_LOG_ERROR("Couldn't run page %d", i);
                    throw pdf_exception("Couldn't run page");
                }
                fz_free_device(dev);

                xref->ctx = main_context;
                data_->lists[i] = list;
            }
        }

        return list;
    }

    bool context::needs_password() const
    {
        return data_->xref && pdf_needs_password(data_->xref);
    }

    bool context::authenticate(std::string const& password)
    {
        LECTOR_LOG_FUNC;
        bool result = false;
        if (data_->xref && needs_password())
            result = pdf_authenticate_password(data_->xref,
                                               const_cast<char*> (password.c_str()));
        return result;
    }

    fz_rect context::get_bbox(std::size_t n)
    {
        pdf_page* page = pdf_load_page(data_->xref, n);
        fz_rect result = pdf_bound_page(data_->xref, page);
        pdf_free_page(ctx_, page);
        return result;
    }

    fz_outline const* context::get_outline()
    {
        if (data_->outline)
            return data_->outline;
        else
        {
            fz_outline* outline = pdf_load_outline(data_->xref);

            boost::mutex::scoped_lock lock(*data_mutex_);
            return data_->outline = outline;
        }
    }

    pixmap context::render_full(float zoom, std::size_t n)
    {
        pdf_page* page = pdf_load_page(data_->xref, n);
        fz_display_list* list = get_display_list(n);
        fz_rect bounds = pdf_bound_page(data_->xref, page);

        fz_matrix ctm = fz_scale(zoom, zoom);

        fz_bbox bbox = fz_round_rect(fz_transform_rect(ctm, bounds));

        pixmap pix (ctx_, bbox, fz_device_rgb);
        // TODO: Clear transparent
        pix.clear(255);

        fz_device* dev = call_ret<fz_device*>(fz_new_draw_device, pix.get());
        fz_execute_display_list(list, dev, ctm, bbox, 0);
        fz_free_device(dev);

        call(pdf_free_page, page);

        return pix;
    }

}
