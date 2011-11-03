#include "pdf_document.hpp"

#include <cmath>

#define PDF_TRY(expr, error_message)            \
    {                                           \
        int error = (expr);                     \
        if (error)                              \
            throw pdf_exception(error_message); \
    }

namespace lector
{

    pdf_document::pdf_document(std::string const& filename,
                               std::string const& password)
    {
        PDF_TRY(pdf_open_xref(&xref_,
                    const_cast<char*> (filename.c_str()),
                    const_cast<char*> (password.c_str())
                ),
                "Could not open document");

        PDF_TRY(pdf_load_page_tree(xref_), "Could not load page tree");

        page_count_ = pdf_count_pages(xref_);
        PDF_TRY(page_count_ == 0, "Empty document");
    }

    pdf_document::~pdf_document()
    {
        pdf_free_xref(xref_);
    }

    pdf_document::find_result_type
        pdf_document::find_next(std::string const& text, std::size_t page)
    {
        for (std::size_t i = page; i < pages(); ++i)
        {
            // TODO: get only text
            pdf_page_ptr page = get_page(i);
            int res = page->find_text(text);
            if (res != -1)
                return find_result_type(true, i, res);
        }
        return find_result_type(false, 0, 0);
    }

    void pdf_document::age_store (std::size_t age)
    {
        pdf_age_store(xref_->store, age);
    }

    pdf_page_ptr pdf_document::get_page (std::size_t index)
    {
        if (index >= pages())
            throw pdf_exception("Invalid page index");

        return pdf_page_ptr(new pdf_page(xref_, index));
    }

    pdf_outline* pdf_document::get_outline()
    {
        return pdf_load_outline(xref_);
    }
    
    std::size_t pdf_document::get_page_number(pdf_link* link)
    {
        if (link->kind == PDF_LINK_GOTO)
            return pdf_find_page_number(xref_, fz_array_get(link->dest, 0));
        else
            return 0;
    }

    std::size_t pdf_document::pages() const
    {
        return page_count_;
    }

}
