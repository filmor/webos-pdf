#include "pixmap_renderer.hpp"

extern "C"
{
#include <fitz.h>
}

namespace lector
{
    pixmap_renderer::pixmap_renderer(fz_context* ctx)
        : colorspace_(fz_device_rgb),
          ctx_(ctx)
    {
        glyphcache_ = fz_new_glyph_cache(ctx_);
        set_antialiasing(8);
        accelerate();
    }

    void pixmap_renderer::accelerate() const
    {
        fz_accelerate();
    }

    void pixmap_renderer::set_antialiasing(std::size_t level) const
    {
        fz_set_aa_level(ctx_, level);
    }

    pixmap pixmap_renderer::render_full(float zoom, pdf_page_ptr page)
    {
        fz_matrix ctm = fz_translate(0, -page->mediabox().y1);
        ctm = fz_concat(ctm, fz_scale(zoom, -zoom));
        ctm = fz_concat(ctm, fz_rotate(page->rotate()));

        fz_bbox bbox = page->get_bbox(ctm);

        pixmap pix (ctx_, bbox, colorspace_);
        pix.clear(255);

        fz_device* dev = fz_new_draw_device(ctx_, glyphcache_, pix.get());
        page->run(dev, ctm, bbox);
        fz_free_device(dev);

        return pix;
    }

    pixmap_renderer::~pixmap_renderer()
    {
        fz_free_glyph_cache(ctx_, glyphcache_);
    }

}
