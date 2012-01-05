#ifndef LECTOR_PIXMAP_RENDERER_HPP
#define LECTOR_PIXMAP_RENDERER_HPP

#include "pdf_document.hpp"
#include "pixmap.hpp"

namespace lector
{

    class pixmap_renderer
    {
    public:
        pixmap_renderer(fz_context*);
        ~pixmap_renderer();

        void accelerate() const;
        void set_antialiasing(std::size_t level) const;

        pixmap render_full(float zoom, pdf_page_ptr page);

    private:
        fz_glyph_cache* glyphcache_;
        fz_colorspace* colorspace_;
        fz_context* ctx_;
    };
}

#endif
