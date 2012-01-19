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

    volatile std::size_t context::xref_count_ = 0;

    context::context(std::size_t limit)
        : xref_(0), page_(0)
    {
        fz_alloc_context* alloc = new fz_alloc_context;

        alloc->malloc = fz_alloc_default.malloc;
        alloc->realloc = fz_alloc_default.realloc;
        alloc->free = fz_alloc_default.free;
        alloc->lock = boost_mutex_lock;
        alloc->unlock = boost_mutex_unlock;
        alloc->user = reinterpret_cast<void*>(&mutex_);

        ctx_ = fz_new_context(alloc, limit << 20);
        list_ = fz_new_display_list(ctx_);
    }

    context::context(context const& other)
        : xref_(other.xref_), page_(0)
    {
        ctx_ = fz_clone_context(const_cast<fz_context*>(other.ctx_));
        syslog(LOG_INFO, "Cloned context");
        // ctx_->alloc->user = reinterpret_cast<void*>(&mutex_);
        syslog(LOG_INFO, "Setup mutex");

        if (other.xref_)
        {
            syslog(LOG_INFO, "Updating count");
            ++xref_count_;
            syslog(LOG_INFO, "Updated count");
            xref_ = new pdf_xref;
            syslog(LOG_INFO, "New xref");
            std::memcpy(xref_, other.xref_, sizeof(pdf_xref));
            syslog(LOG_INFO, "memcpy");
            xref_->ctx = ctx_;
            syslog(LOG_INFO, "Done updating");
        }

        syslog(LOG_INFO, "Copying list");
        list_ = fz_new_display_list(ctx_);
        syslog(LOG_INFO, "Copied list");
    }
   
    context::~context()
    {
        if (page_)
            pdf_free_page(ctx_, page_);

        if (xref_)
        {
            boost::mutex::scoped_lock lock(mutex_);

            if (--xref_count_ == 0)
                pdf_free_xref(xref_);
            else
                delete xref_;
        }

        fz_free_outline(outline_);
        fz_free_display_list(ctx_, list_);
        fz_free_context(ctx_);
    }

    // Move this into the constructor
    void context::load_file(std::string const& name)
    {
        if (xref_ && xref_count_ > 0)
            throw fitz_exception("AYEE!");

        fz_try(ctx_)
        {
            boost::mutex::scoped_lock lock(mutex_);
            xref_count_++;
            lock.unlock();
            xref_ = pdf_open_xref(ctx_, const_cast<char*> (name.c_str()));
        }
        fz_catch(ctx_)
        {
            xref_ = 0;
            throw pdf_exception("Couldn't open PDF file");
        }
        syslog(LOG_INFO, "Done loading page");

        // TODO: This does only work if the file doesn't need a password. Do
        //       this in "authenticate" or an additional private function
        fz_try(ctx_)
        {
            page_count_ = pdf_count_pages(xref_);
        }
        fz_catch(ctx_)
        {
            pdf_free_xref(xref_);
            xref_ = 0;
            throw pdf_exception("Couldn't load page tree");
        }
    }

    bool context::needs_password() const
    {
        return xref_ && pdf_needs_password(xref_);
    }

    bool context::authenticate(std::string const& password)
    {
        if (xref_)
            return pdf_authenticate_password(xref_, 
                                             const_cast<char*> (password.c_str()));
        else
            return false;
    }

    void context::load_page(std::size_t n)
    {
        if (page_)
            pdf_free_page(ctx_, page_);
        page_ = pdf_load_page(xref_, n);

        fz_device* device = fz_new_list_device(ctx_, list_);

        fz_try(ctx_)
        {
            pdf_run_page(xref_, page_, device, fz_identity, 0);
        }
        fz_catch(ctx_)
        {
            throw pdf_exception("Couldn't load page");
        }

        fz_free_device(device);
    }

    fz_rect context::get_bbox()
    {
        return pdf_bound_page(xref_, page_);
    }

    fz_outline const* context::get_outline()
    {
        if (outline_)
            return outline_;
        else
            return outline_ = pdf_load_outline(xref_);
    }

}
