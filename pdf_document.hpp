#ifndef PDF_DOCUMENT_HPP
#define PDF_DOCUMENT_HPP

#include <string>
#include <stdexcept>
#include <vector>
#include <memory>
#include <tuple>
#include <boost/utility.hpp>

extern "C"
{
#include <fitz.h>
#include <mupdf.h>
}

namespace lector
{

    struct pdf_exception : std::runtime_error
    {
        pdf_exception(const char* message) : std::runtime_error(message)
        {}
    };

    class pdf_page;
    typedef std::shared_ptr<pdf_page> pdf_page_ptr;
    
    class pdf_document
    {
    public:
        typedef std::tuple<bool, std::size_t, std::size_t> find_result_type;

        // TODO Use streams
        pdf_document(fz_context* ctx,
                     std::string const& filename,
                     std::string const& password = "");
        ~pdf_document();

        find_result_type
            find_next(std::string const& text, std::size_t page);

        fz_outline* get_outline();

        pdf_page_ptr get_page (std::size_t index);
        std::size_t pages() const;

    private:
        pdf_xref* xref_;
        fz_context* context_;
        std::size_t page_count_;
    };

    class pdf_page : boost::noncopyable
    {
    private:
        friend class pdf_document;
        pdf_page(pdf_xref* xref, std::size_t n);

    public:
        ~pdf_page();
        
        std::size_t height() const;
        std::size_t width() const;
        fz_bbox get_bbox(fz_matrix const& matrix) const;
        fz_rect const& mediabox() const { return mediabox_; }

        int rotate() const { return rotate_; }

        void run(fz_device* device, fz_matrix const& matrix,
                 fz_bbox const& bbox) const;

        int find_text(std::string const&, std::size_t start = 0) const;

    private:
        pdf_xref* xref_;
        fz_rect mediabox_;
        int rotate_;
        fz_display_list* list_;
        fz_text_span* text_span_;
    };

}

#endif
