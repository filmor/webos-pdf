#include "pixmap_renderer.hpp"

extern "C"
{
#include <fitz.h>
}

namespace viewer
{
    pixmap_renderer::pixmap_renderer()
        : colorspace_(fz_device_rgb)
    {
        glyphcache_ = fz_new_glyph_cache();
        set_antialiasing(8);
        accelerate();
    }

    void pixmap_renderer::accelerate() const
    {
        fz_accelerate();
    }

    void pixmap_renderer::set_antialiasing(std::size_t level) const
    {
        fz_set_aa_level(level);
    }

    pixmap pixmap_renderer::render_full(float zoom, pdf_page const& page)
    {
        fz_matrix ctm = fz_translate(0, -int(page.height()));
        ctm = fz_concat(ctm, fz_scale(zoom, -zoom));
        ctm = fz_concat(ctm, fz_rotate(page.rotate()));

        fz_bbox bbox = page.get_bbox(ctm);

        const std::size_t width = bbox.x1 - bbox.x0;
        const std::size_t height = bbox.y1 - bbox.y0;
        pixmap pix (width, height, colorspace_);
        pix.clear(255);



        fz_device* dev = fz_new_draw_device(glyphcache_, pix.get());
        page.run(dev, ctm, bbox);
        fz_free_device(dev);

        return pix;
    }

    pixmap_renderer::~pixmap_renderer()
    {
    }

}
