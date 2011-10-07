#include <SDL.h>
#include <GLES/gl2.h>
#include <unistd.h>
#include <mupdf.h>
#include <fitz.h>

#define EXIT_ASSERT(exp, str)       \
    if (!(exp))                     \
    {                               \
        printf("Error: %s\n", str);   \
        return 1;                   \
    }

int main() (int argc, char** argv)
{
    EXIT_ASSERT(argc == 2, "use (name) file");

    // PDF file
    pdf_xref* xref;
    int error = pdf_open_xref(&xref, argv[1], "");
    EXIT_ASSERT(error == 0, "couldn't open file");
    pdf_load_page_tree(xref);

    // Page
    printf("Showing page 1 of %d\n", pdf_count_pages(xref));
    pdf_page* page;
    error = pdf_load_page(&page, xref, 0);
    
    
    int error = SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_SetVideoMode(0, 0, 0, SDL_OPENGL);



}
