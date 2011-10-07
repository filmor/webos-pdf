#include <poppler/glib/poppler.h>
#include <SDL/SDL.h>
// #include <PDL.h>
#include <cairo.h>
#include "cairosdl/cairosdl.h"
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
    SDL_Surface* screen = SDL_SetVideoMode(0, 0, 0, SDL_SWSURFACE);
//    PDL_Init(0);
    
    while (SDL_LockSurface(screen) != 0)
        SDL_Delay(1);

    cairo_t* context = cairosdl_create(screen);
    poppler_page_render(page, context);
    cairo_destroy(context);

    SDL_UnlockSurface(screen);

    SDL_Flip(screen);

    SDL_Event ev;
    while (SDL_WaitEvent(&ev))
    {
        if (ev.type == SDL_Quit)
            break;
    }
    
//    PDL_Quit();
    
    g_object_unref(page);
    g_object_unref(doc);
}
