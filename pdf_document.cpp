#include "pdf_document.hpp"

#define PDF_TRY(expr, error_message)            \
    {                                           \
        int error = (expr);                     \
        if (error)                              \
            throw pdf_exception(error_message); \
    }

namespace viewer
{

    pdf_document::pdf_document(std::string const& filename,
                               std::string const& password = "")
    {
        PDF_TRY(pdf_open_xref(&xref_, filename.c_str(), password.c_str()),
                "Could not open document " + filename);

        PDF_TRY(pdf_load_page_tree(&xref_), "Could load page tree of " + filename);
    }

    std::size_t pdf_document::pages() const
    {
        return pdf_count_pages(xref_);
    }


    pdf_page::pdf_page(pdf_document& doc, std::size_t n)
    {
        PDF_TRY(pdf_load_page(&page_, doc.xref_, n),
                "Could not load page");

        // Create display list
        list_ = fz_new_display_list();
        fz_device* dev = fz_new_list_device(list_);
        PDF_TRY(pdf_run_page(doc.xref_, n, dev, fz_identity),
                "Could not draw page");
        fz_free_device(dev);

        // Load text
        text_span_ = fz_new_text_span();
        dev = fz_new_text_device(text_span_);
        fz_execute_display_list(list_, dev, fz_identity, fz_infinite_bbox);
        fz_free_device(dev);

        fz_text_span* span = text;

        do
        {
            text_.append(span->text, span->len);
            span = span->next;
        }
        while (span);

        
    }
}
