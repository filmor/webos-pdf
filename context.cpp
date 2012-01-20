#include "context.hpp"

#include <cstring>
#include <syslog.h>

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

        fz_outline* outline;
        pdf_xref* xref;
        fz_display_list** lists;
        std::size_t page_count;

        fz_alloc_context* alloc;
    };
    
    context::context(std::size_t limit)
        : data_(new shared_data)
    {
        data_->alloc->malloc = fz_alloc_default.malloc;
        data_->alloc->realloc = fz_alloc_default.realloc;
        data_->alloc->free = fz_alloc_default.free;
        data_->alloc->lock = boost_mutex_lock;
        data_->alloc->unlock = boost_mutex_unlock;

        data_->alloc->user = reinterpret_cast<void*>(mutex_.get());
        data_->lists = 0;

        ctx_ = fz_new_context(data_->alloc, limit << 20);
    }

    context::context(context const& other)
        : data_(other.data_)
    {
        ctx_ = fz_clone_context(const_cast<fz_context*>(other.ctx_));
        syslog(LOG_INFO, "Cloned context");
    }
   
    context::~context()
    {
        boost::mutex::scoped_lock lock(*mutex_);
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
        fz_display_list** lists;
        pdf_xref* xref = 0;

        fz_try(ctx_)
        {
            xref = pdf_open_xref(ctx_, const_cast<char*> (name.c_str()));
        }
        fz_catch(ctx_)
        {
            xref = 0;
            throw pdf_exception("Couldn't open PDF file");
        }
        syslog(LOG_INFO, "Done loading page");

        // TODO: This does only work if the file doesn't need a password. Do
        //       this in "authenticate" or an additional private function
        fz_try(ctx_)
        {
            std::size_t page_count = pdf_count_pages(data_->xref);

            lists = new fz_display_list*[page_count];

            // Create display lists
            for (unsigned i = 0; i < page_count; ++i)
            {
                lists[i] = fz_new_display_list(ctx_);

                fz_try(ctx_)
                {
                    pdf_page* page = pdf_load_page(xref, i);
                    fz_device* dev = fz_new_list_device(ctx_, lists[i]);
                    pdf_run_page(xref, page, dev, fz_identity, 0);
                    fz_free_device(dev);
                }
                fz_catch(ctx_)
                {
                    syslog(LOG_ERR, "Couldn't create list for page %d", i);
                }
            }

            data_->recycle(ctx_);

            {
                // Atomic write of data_
                boost::mutex::scoped_lock lock(*mutex_);

                data_->xref = xref;
                data_->page_count = page_count;
                data_->lists = lists;
            }
        }
        fz_catch(ctx_)
        {
            pdf_free_xref(xref);
            xref = 0;
            throw pdf_exception("Couldn't load pages");
        }
    }

    bool context::needs_password() const
    {
        return data_->xref && pdf_needs_password(data_->xref);
    }

    bool context::authenticate(std::string const& password)
    {
        if (data_->xref)
            return pdf_authenticate_password(data_->xref,
                                             const_cast<char*> (password.c_str()));
        else
            return false;
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

            boost::mutex::scoped_lock lock(*mutex_);
            return data_->outline = outline;
        }
    }

    pixmap context::render_full(float zoom, std::size_t n)
    {
        pdf_page* page = pdf_load_page(data_->xref, n);
        fz_matrix ctm = fz_translate(0, -page->mediabox.y1);
        ctm = fz_concat(ctm, fz_scale(zoom, -zoom));
        ctm = fz_concat(ctm, fz_rotate(page->rotate));

        fz_bbox bbox = fz_round_rect(fz_transform_rect(ctm, page->mediabox));

        pixmap pix (ctx_, bbox, fz_device_rgb);
        // TODO: Clear transparent
        pix.clear(255);

        fz_device* dev = fz_new_draw_device(ctx_, pix.get());
        fz_execute_display_list(data_->lists[n], dev, ctm, bbox, 0);
        fz_free_device(dev);

        pdf_free_page(ctx_, page);

        return pix;
    }

}
