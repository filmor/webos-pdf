
#include <cstdlib>

int main(int argc, char** argv)
{
    if (argc != 2)
        std::exit(1);

    const char* filename = argv[1];

    pdf_document doc (filename);
    // The index operator is on-demand
    // Prepare pages by doc.prepare(from, to)
    pdf_page& page = doc[1];

    // Derived from renderer which handles the glyph cache
    png_renderer renderer;

    double zoom = 1.0;
    renderer.render_full(zoom, page, std::string(filename) + ".png");

    // Thumbnail:
    renderer.render_full(zoom, page, std::string(filename) + "-thumb.png");

    // TODO: GLES texture
    // TODO: Implement this stuff
    // TODO: more rendering options (especially partial rendering)
    // TODO: Absolute width or height values instead of zoom

