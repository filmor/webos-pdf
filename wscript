from os.path import join, basename

VERSION='0.1'
APPNAME='lector'

PDK_PATH="/opt/PalmPDK"
# Relative to build
MUPDF_PATH="../mupdf"

def options(opt):
    opt.load('compiler_cxx boost')

def configure(conf):
    mode = "release"

    conf.load('compiler_cxx boost')
    for i in "-Wall -g3 -O2 -std=c++0x".split():
        conf.env.append_value('CXXFLAGS', i)

    # PDK
    conf.env.append_value("INCLUDES_PDK", join(PDK_PATH, "include"))
    conf.env.append_value("LIBPATH_PDK", join(PDK_PATH, "device", "lib"))
    conf.env.append_value("LIB_PDK", ["pdl", "z"])

    # SDL
    conf.env.append_value("INCLUDES_SDL", join(PDK_PATH, "include", "SDL"))
    conf.env.append_value("LIBPATH_SDL", join(PDK_PATH, "device", "lib"))
    conf.env.append_value("DEFINES_SDL", ["_GNU_SOURCE=1", "_REENTRANT"])
    conf.env.append_value("LIB_SDL", ["SDL", "pthread"])

    # GL
    conf.env.append_value("INCLUDES_GL", join(PDK_PATH, "include"))
    conf.env.append_value("LIBPATH_GL", join(PDK_PATH, "device", "lib"))
    conf.env.append_value("LIB_GL", ["GLESv2"])

    # FITZ
    conf.env.append_value("INCLUDES_FITZ", join(MUPDF_PATH, "fitz"))
    conf.env.append_value("STLIBPATH_FITZ", join(MUPDF_PATH, "build", mode))
    conf.env.append_value("STLIB_FITZ", "fitz")

    # MUPDF
    conf.env.append_value("INCLUDES_MUPDF", join(MUPDF_PATH, "pdf"))
    conf.env.append_value("STLIBPATH_MUPDF", join(MUPDF_PATH, "build", mode))
    conf.env.append_value("STLIB_MUPDF",
                          ["mupdf", "fitz", "jbig2dec", "openjpeg"]
                         )
    conf.env.append_value("LIB_MUPDF", ["jpeg", "freetype"])

    conf.check_boost()

def build(bld):
    # TODO: Proper dependencies
    add_cpp = lambda *args: [ i + ".cpp" for i in args ]

    common = add_cpp("pdf_document", "pdf_page", "pixmap_renderer")
    bld.objects(source = common,
            use = "MUPDF FITZ BOOST SDL PDK", target = "common"
            )

    programs = {
                 "arxservice" :
                    (add_cpp("service", "util/filesystem"), []),
                 "lector" : 
                    (add_cpp("gles", "renderer", "gles/program"), ["GL"])
               }

    for prog, t in programs.items():
        bld.program(source=t[0],
                    use=["common"] + t[1],
                    target=prog)

