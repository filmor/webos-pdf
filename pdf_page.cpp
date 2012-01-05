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
    pdf_page::pdf_page(pdf_xref* xref, std::size_t n)
    {
        xref_ = xref;
        ::pdf_page* page = pdf_load_page(xref, n);

        // Create display list
        list_ = fz_new_display_list(xref->ctx);
        fz_device* dev = fz_new_list_device(xref->ctx, list_);
        pdf_run_page(xref, page, dev, fz_identity, 0);
        fz_free_device(dev);

        text_span_ = fz_new_text_span(xref->ctx);
        dev = fz_new_text_device(xref->ctx, text_span_);
        fz_execute_display_list(list_, dev, fz_identity, fz_infinite_bbox, 0);
        fz_free_device(dev);

        mediabox_ = page->mediabox;
        rotate_ = page->rotate;
        pdf_free_page(xref->ctx, page);
    }

    fz_bbox pdf_page::get_bbox(fz_matrix const& matrix) const
    {
        return fz_round_rect(fz_transform_rect(matrix, mediabox_));
    }

    void pdf_page::run(fz_device* device, fz_matrix const& matrix,
                       fz_bbox const& bbox) const
    {
        fz_execute_display_list(list_, device, matrix, bbox, 0);
    }

    std::size_t pdf_page::height() const
    {
        return std::abs(mediabox_.y1 - mediabox_.y0);
    }

    std::size_t pdf_page::width() const
    {
        return std::abs(mediabox_.x1 - mediabox_.x0);
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
        fz_free_text_span(xref_->ctx, text_span_);
        fz_free_display_list(xref_->ctx, list_);
    }
    
}
