#include "png_renderer.hpp"

namespace viewer
{
    png_renderer::png_renderer()
        : colorspace_(fz_device_rgb)
    {
        fz_accelerate();
        fz_set_aa_level(8);

        glyphcache_ = fz_new_glyph_cache();
    }


    void png_renderer::render_full(float zoom, pdf_page const& page,
                                   std::string const& filename)
    {
        fz_matrix ctm = fz_translate(0, -(int)page.height());
        ctm = fz_concat(ctm, fz_scale(zoom, -zoom));
        ctm = fz_concat(ctm, fz_rotate(page.rotate()));

        fz_bbox bbox = page.get_bbox(ctm);

        fz_pixmap* pix = fz_new_pixmap_with_rect(colorspace_, bbox);
        fz_clear_pixmap_with_color(pix, 255);

        fz_device* dev = fz_new_draw_device(glyphcache_, pix);

        page.run(dev, ctm, bbox);
        fz_free_device(dev);

        fz_write_png(pix, const_cast<char*> (filename.c_str()), 0);

        fz_drop_pixmap(pix);
    }

    png_renderer::~png_renderer()
    {
        fz_free_glyph_cache(glyphcache_);
    }

}
