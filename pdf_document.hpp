#ifndef PDF_DOCUMENT_HPP
#define PDF_DOCUMENT_HPP

#include <string>
#include <stdexcept>
#include <vector>
#include <utility>
#include <tuple>

extern "C"
{
#include <fitz.h>
#include <mupdf.h>
}

namespace viewer
{

    struct pdf_exception : std::runtime_error
    {
        pdf_exception(const char* message) : std::runtime_error(message)
        {}
    };

    class pdf_page;
    
    class pdf_document
    {
    public:
        friend class pdf_page;
        typedef std::tuple<bool, std::size_t, std::size_t> find_result_type;

        // TODO Use streams
        pdf_document(std::string const& filename,
                     std::string const& password = "")
            ;
        ~pdf_document();

        find_result_type
            find_next(std::string const& text, std::size_t page);
        

        pdf_page& operator[] (std::size_t index);

        std::size_t pages() const;

    private:
        pdf_xref* xref_;
        std::vector<pdf_page*> pages_;
    };

    class pdf_page
    {
    public:
        pdf_page(pdf_document& doc, std::size_t n);
        ~pdf_page();

        std::size_t height() const;
        std::size_t width() const;
        fz_bbox get_bbox(fz_matrix const& matrix) const;
        int rotate() const { return page_->rotate; }

        void run(fz_device* device, fz_matrix const& matrix,
                 fz_bbox const& bbox) const;

        int find_text(std::string const&, std::size_t start = 0) const;

    private:
        pdf_document& doc_;
        ::pdf_page* page_;
        fz_display_list* list_;
        fz_text_span* text_span_;
    };

}

#endif
