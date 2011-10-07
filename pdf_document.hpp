#ifndef PDF_DOCUMENT_HPP
#define PDF_DOCUMENT_HPP

#include <string>
#include <stdexcept>

#include "mupdf/mupdf.h"
#include "fitz/fitz.h"

namespace viewer
{

    class pdf_exception : public runtime_error
    {
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

        std::size_t pages() const;

    private:
        pdf_xref* xref_;
    };

    class pdf_page
    {
    public:
        pdf_page(pdf_document& doc, std::size_t n);
        ~pdf_page();

    private:
        pdf_document& doc_;
        pdf_page* page_;
        fz_display_list* list_;
        fz_text_span* text_span_;
        std::string text_:
    };

}

#endif
