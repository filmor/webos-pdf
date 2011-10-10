#include "pdf_document.hpp"

#include <cmath>

#define PDF_TRY(expr, error_message)            \
    {                                           \
        int error = (expr);                     \
        if (error)                              \
            throw pdf_exception(error_message); \
    }

namespace viewer
{

    pdf_document::pdf_document(std::string const& filename,
                               std::string const& password)
    {
        PDF_TRY(pdf_open_xref(&xref_,
                    const_cast<char*> (filename.c_str()),
                    const_cast<char*> (password.c_str())
                ),
                "Could not open document");

        PDF_TRY(pdf_load_page_tree(xref_), "Could load page tree");

        pages_.resize(pdf_count_pages(xref_));
    }

    std::string pdf_document::unique_id() const
    {

    }

    pdf_document::~pdf_document()
    {
        pdf_free_xref(xref_);
    }

    pdf_page& pdf_document::operator[] (std::size_t index)
    {
        if (index >= pages_.size())
            throw pdf_exception("Invalid page index");

        if (pages_[index] == 0)
            pages_[index] = new pdf_page(*this, index);

        return *pages_[index];
    }

    std::size_t pdf_document::pages() const
    {
        return pages_.size();
    }

    pdf_page::pdf_page(pdf_document& doc, std::size_t n) : doc_(doc)
    {
        PDF_TRY(pdf_load_page(&page_, doc.xref_, n),
                "Could not load page");

        // Create display list
        list_ = fz_new_display_list();
        fz_device* dev = fz_new_list_device(list_);
        PDF_TRY(pdf_run_page(doc.xref_, page_, dev, fz_identity),
                "Could not draw page");
        fz_free_device(dev);

        // Load text
        text_span_ = fz_new_text_span();
        dev = fz_new_text_device(text_span_);
        fz_execute_display_list(list_, dev, fz_identity, fz_infinite_bbox);
        fz_free_device(dev);

        fz_text_span* span = text_span_;
    }

    fz_bbox pdf_page::get_bbox(fz_matrix const& matrix) const
    {
        return fz_round_rect(fz_transform_rect(matrix, page_->mediabox));
    }

    void pdf_page::run(fz_device* device, fz_matrix const& matrix,
                       fz_bbox const& bbox) const
    {
        fz_execute_display_list(list_, device, matrix, bbox);
    }

    std::size_t pdf_page::height() const
    {
        return std::abs(page_->mediabox.y1 - page_->mediabox.y0);
    }

    std::size_t pdf_page::width() const
    {
        return std::abs(page_->mediabox.x1 - page_->mediabox.x0);
    }

    pdf_page::~pdf_page()
    {
        fz_free_text_span(text_span_);
        fz_free_display_list(list_);
        pdf_free_page(page_);
    }
}
