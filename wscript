VERSION='0.1'
APPNAME='arxservice'

def options(opt):
    pass

def configure(conf):
    for i in """-Wall -g3""".split():
        conf.env.append_value('CXXFLAGS', i)

    conf.load('compiler_cxx boost')
    conf.check_boost(lib="format")

def build(bld):
    service = "service.cpp"
    test = "test.cpp"
    bld.objects(
            source = [i for i in bld.path.ant_glob("*.cpp") if not i in mains],
            use = "BOOST", target = "objs"
            )
    for main in mains:
        bld.program(source=str(main), use="BOOST objs", target=str(main)[5:-4])

