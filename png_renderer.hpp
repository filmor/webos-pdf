#ifndef PNG_RENDERER_HPP
#define PNG_RENDERER_HPP

#include "pdf_document.hpp"

namespace viewer
{

    class png_renderer
    {
    public:
        png_renderer(fz_glyph_cache* glyphcache);
        ~png_renderer();

        void render_full(float zoom, pdf_page const& page,
                         std::string const& filename);

    private:
        fz_glyph_cache* glyphcache_;
        fz_colorspace* colorspace_;
    };
}

#endif
