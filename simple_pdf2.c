#include <poppler/glib/poppler.h>
#include <SDL/SDL.h>
// #include <PDL.h>
#include <cairo.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    g_type_init();

    if (argc != 2)
    {
        printf("Moep!\n");
        return 1;
    }

    GError* err = NULL;

    PopplerDocument* doc = poppler_document_new_from_file(argv[1], NULL, &err);
    if (!doc) {
        printf("Moep, Datei kaputt: %s\n", err->message);
        return 2;
    }

    PopplerPage* page = poppler_document_get_page(doc, 0);
    if (!page) {
        printf("Moep, Seite 1 nicht da\n");
        g_object_unref(err);
        return 3;
    }

    int pages = poppler_document_get_n_pages(doc);
    printf("Showing page 1 of %d.\n", pages);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface* surface = SDL_SetVideoMode(0, 0, 0, SDL_SWSURFACE);
//    PDL_Init(0);
    
/*    SDL_Surface* surface = SDL_CreateRGBSurface(
            flags, width, height, 32,
            0x00ff0000,
            0x0000ff00,
            0x000000ff,
            0);*/

    cairo_surface_t *cairosurf = cairo_image_surface_create_for_data (
        surface->pixels,
        CAIRO_FORMAT_RGB24,
        surface->w,
        surface->h,
        surface->pitch
        );

    cairo_t* context = cairo_create(cairosurf);
    poppler_page_render(page, context);
    cairo_destroy(context);

    SDL_Flip(surface);

    cairo_surface_write_to_png(cairosurf, "bla.png");

    SDL_Event ev;
    while (SDL_WaitEvent(&ev))
    {
        if (ev.type == SDL_Quit)
            break;
    }
    
//    PDL_Quit();
    
    cairo_surface_destroy(cairosurf);
    g_object_unref(page);
    g_object_unref(doc);
}
