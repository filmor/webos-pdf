#ifndef PDF_DOCUMENT_HPP
#define PDF_DOCUMENT_HPP

#include <string>
#include <stdexcept>
#include <vector>

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

        // TODO Use streams
        pdf_document(std::string const& filename,
                     std::string const& password = "")
            ;
        ~pdf_document();

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
        fz_bbox get_bbox(fz_matrix const& matrix) const;
        int rotate() const { return page_->rotate; }

        void run(fz_device* device, fz_matrix const& matrix,
                 fz_bbox const& bbox) const;

        void run(fz_device* device, fz_matrix const& matrix) const
        {
            run(device, matrix, get_bbox(matrix));
        }


    private:
        pdf_document& doc_;
        ::pdf_page* page_;
        fz_display_list* list_;
        fz_text_span* text_span_;
    };

}

#endif
