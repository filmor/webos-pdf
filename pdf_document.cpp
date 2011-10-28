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

        PDF_TRY(pdf_load_page_tree(xref_), "Could not load page tree");

        const std::size_t page_count = pdf_count_pages(xref_);
        PDF_TRY(page_count == 0, "Empty document");

        pages_.resize(page_count);
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
            pdf_page const& page = (*this)[i];
            int res = page.find_text(text);
            if (res != -1)
                return find_result_type(true, i, res);
        }
        return find_result_type(false, 0, 0);
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

        text_span_ = fz_new_text_span();
        dev = fz_new_text_device(text_span_);
        fz_execute_display_list(list_, dev, fz_identity, fz_infinite_bbox);
        fz_free_device(dev);
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

    int pdf_page::find_text(std::string const& text, std::size_t start) const
    {
        fz_text_span* span = text_span_;
        std::size_t offset = 0;
        std::size_t match_index = 0;
        while (span)
        {
            // TODO: Check for span->eol and, if that's the case add 1 to offset
            //       and act like there is a ' ' at the end of the line
            for (std::size_t i = 0; i < std::size_t(span->len); ++i)
            {
                // This could be optimised by doing some checking before
                // starting the loop
                if (offset + i < start)
                    continue;

                if (span->text[i].c == text[match_index])
                    ++match_index;

                if (match_index == text.size())
                    return offset + i - text.size();
            }
            offset += span->len;
            span = span->next;
        }
        return -1;
    }

    pdf_page::~pdf_page()
    {
        fz_free_text_span(text_span_);
        fz_free_display_list(list_);
        pdf_free_page(page_);
    }
}
