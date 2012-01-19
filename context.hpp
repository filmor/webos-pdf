#ifndef LECTOR_CONTEXT_HPP
#define LECTOR_CONTEXT_HPP

#include <stdexcept>
#include <memory>
#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>

extern "C"
{
#include <fitz.h>
#include <mupdf.h>
}

#include "pixmap.hpp"

namespace lector
{

    struct fitz_exception : std::runtime_error
    {
        fitz_exception(const char* message)
            : std::runtime_error(message)
        {}
    };

    struct pdf_exception : fitz_exception
    {
        pdf_exception(const char* message)
            : fitz_exception(message)
        {}
    };

    // TODO: Merge pixmap_renderer, pdf_document and pdf_page into context
    //       Maybe only merge the first two, pdf_page actually has some use
    //
    // TODO: Rename to draw_context, add a text_context
    // 
    class context
    {
    public:
        context(std::size_t limit = 512);
        context(context const& other);
        ~context();

        bool authenticate(std::string const&);
        bool needs_password() const;

        void load_file(std::string const& filename);

        void load_page(std::size_t n);
        fz_outline const* get_outline();

        std::size_t get_page_count() const { return page_count_; }

        fz_rect get_bbox();
        pixmap render_full(float zoom);

        template <typename Func, typename... Args>
        void call (Func func, Args... args)
        {
            fz_try(ctx_)
            {
                func(ctx_, args...);
            }
            fz_catch(ctx_)
            {
                // Throw C++ exception
            }
        }

    private:
        boost::mutex mutex_;
        fz_context* ctx_;
        
        pdf_xref* xref_;
        std::size_t page_count_;
        fz_outline* outline_;

        pdf_page* page_;
        fz_display_list* list_;

        static volatile std::size_t xref_count_;
    };

}

#endif
