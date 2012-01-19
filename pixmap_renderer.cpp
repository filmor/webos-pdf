#include "context.hpp"

extern "C"
{
#include <fitz.h>
}

namespace lector
{

#if 0
    pixmap context::render_square(std::size_t x, std::size_t y,
                                  std::size_t size, float zoom_factor = 1.0f)
    {
        fz_bbox bbox = {x, x + size, y, y + size};
        return render(page, bbox, zoom_factor, true);
    }

    pixmap context::render(fz_bbox bbox, float zoom_factor = 1.0f,
                           bool clamp = false)
    {
        fz_matrix ctm = fz_translate(0, -page->mediabox(), y1);
        ctm = fz_concat(ctm, fz_scale(zoom_factor, -zoom_factor));
        ctm = fz_concat(ctm, fz_rotate(page->rotate()));

        if (clamp)
        {
            fz_bbox page_bbox = page->get_bbox(ctm);
            bbox.x1 = std::min(page_bbox.x1, bbox.x1);
            bbox.x2 = std::max(page_bbox.x2, bbox.x2);
            bbox.y1 = std::min(page_bbox.y1, bbox.y1);
            bbox.y2 = std::max(page_bbox.y2, bbox.y2);
        }

        pixmap pix (ctx_, bbox, colorspace_);
        // for tiles we actually need the transparency
        // pix.clear(255);
        pix.clear(0);

        // TODO: Look up if draw_device is thread-safe.
        // Done: It isn't!
        fz_device* dev = fz_new_draw_device(ctx_, pix.get());
        page->run(dev, ctm, bbox);
        fz_free_device(dev);

        return pix;
    }
#endif

    pixmap context::render_full(float zoom)
    {
        fz_matrix ctm = fz_translate(0, -page_->mediabox.y1);
        ctm = fz_concat(ctm, fz_scale(zoom, -zoom));
        ctm = fz_concat(ctm, fz_rotate(page_->rotate));

        fz_bbox bbox = fz_round_rect(fz_transform_rect(ctm, page_->mediabox));

        pixmap pix (ctx_, bbox, fz_device_rgb);
        // TODO: Clear transparent
        pix.clear(255);

        fz_device* dev = fz_new_draw_device(ctx_, pix.get());
        fz_execute_display_list(list_, dev, ctm, bbox, 0);
        fz_free_device(dev);

        return pix;
    }

}
