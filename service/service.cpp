bool open(const char* source)
{
}

PDL_bool handler(PDL_JSParameters* params)
{
    int argc = PDL_GetNumJSParams(params);

}

const char version_information[] = "mupdf 0.9"

int main()
{
    PDL_Init();
    atexit(PDL_Quit);
    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    PDL_RegisterJSHandler("Handler", &handler);

    PDL_JSRegistrationComplete();

    PDL_CallJS("VersionCallback", &version_information, 1);

}
