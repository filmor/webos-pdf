#include <string>
#include <cstdlib>

#include "pdf_document.hpp"
#include "png_renderer.hpp"

using namespace viewer;

int main(int argc, char** argv)
{
    if (argc != 2)
        std::exit(1);

    const char* filename = argv[1];

    pdf_document doc (filename);
    // The index operator is on-demand
    // Prepare pages by doc.prepare(from, to)
    viewer::pdf_page& page = doc[0];

    // Derived from renderer which handles the glyph cache
    png_renderer renderer;

    renderer.render_full(2.28, page, std::string(filename) + ".png");

    // TODO: GLES texture
    // TODO: more rendering options (especially partial rendering)
    // TODO: Absolute width or height values instead of zoom
}
