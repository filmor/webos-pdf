from os.path import join, basename

VERSION='0.1'
APPNAME='arxservice'

PDK_PATH="/opt/PalmPDK"
# Relative to build
MUPDF_PATH="../mupdf"

def options(opt):
    opt.load('compiler_cxx boost')

def configure(conf):
    mode = "release"

    conf.load('compiler_cxx boost')
    for i in "-Wall -g3 -O2".split():
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

    # FITZ
    conf.env.append_value("INCLUDES_FITZ", join(MUPDF_PATH, "fitz"))
    conf.env.append_value("STLIBPATH_FITZ", join(MUPDF_PATH, "build", mode))
    conf.env.append_value("STLIB_FITZ", "fitz")

    # MUPDF
    conf.env.append_value("INCLUDES_MUPDF", join(MUPDF_PATH, "pdf"))
    conf.env.append_value("STLIBPATH_MUPDF", join(MUPDF_PATH, "build", mode))
    conf.env.append_value("STLIB_MUPDF",
                          ["mupdf", "fitz", "jbig2dec", "jpeg", "openjpeg", "freetype"]
                         )

    conf.check_boost()

def build(bld):
    service = "service.cpp"
    test = "test.cpp"
    gles = "gles.cpp"
    mains = [test, service, gles]
    sources = [i for i in bld.path.ant_glob("*.cpp") if not basename(str(i)) in mains]
    bld.objects(source = sources,
            use = "MUPDF FITZ PDK SDL BOOST", target = "objs"
            )
    for main in mains:
        bld.program(source=str(main), use="BOOST objs", target="arx" + str(main)[:-4])

