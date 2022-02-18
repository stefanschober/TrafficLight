#! /usr/bin/env python3
import argparse, subprocess, json, sys, os, re

# data dictionary of default values
# key: [toolchain, container, generator, extra_config]
platforms = {
    'Gnu': {
        'toolchain': 'gnuarm',
        'generator': 'Make',
        'extracfg': []
    },
    'Pico': {
        'toolchain': 'gnuarm',
        'generator': 'Make',
        'extracfg': ['CONFIG_KERNEL_QK=TRUE', 'CONFIG_PICO=TRUE']
    },
    'Arm': {
        'toolchain': 'armcc',
        'generator': 'Make',
        'extracfg': []
    },
    'Raspi': {
        'toolchain': 'gnuarmhf',
        'generator': 'Make',
        'extracfg': ['CONFIG_RASPI=ON', 'CONFIG_RASPI_IO=ON', 'CONFIG_PIGPIO=ON', 'CONFIG_GUI=ON']
    },
    'MinGW': {
        'toolchain': 'mingwgcc',
        'generator': 'Make',
        'extracfg': ['CONFIG_GUI=ON']
    },
    'Linux': {
        'toolchain': None,
        'generator': 'Make',
        'extracfg': ['CONFIG_GUI=ON']
    },
}
generators = {
    'Make': 'Unix Makefiles',
    'Ninja': 'Ninja'
}
containers = {
    'crosscompile': ['Gnu', 'Pico', 'Raspi', 'MinGW'],
    'keil': ['Arm'],
    'none': ['Linux']
}

globalcfg=[]
myscript = None
mydir = None
myapp = None


# functions
def intersection(a, b) -> list:
    return list(set(a) & set(b))


def read_config() -> bool:
    global platforms, generators, containers, globalcfg
    ret = False
    cfgfiles = [f"/etc/buildprj.cfg", f"{os.environ['HOME']}/.config/buildprj/default.cfg", f"{os.environ['HOME']}/.buildprj", f"{os.getcwd()}/buildprj.cfg"]

    for fn in cfgfiles:
        if(os.path.isfile(fn)):
            try:
                fp = open(fn, "r")
            except EnvironmentError as e:
                print(f"cannot open configuration ({fn}")
                continue
            else:
                cfg = json.load(fp)
                fp.close()
                for f in ['platforms', 'generators', 'containers', 'globalcfg']:
                    if f in cfg:
                        globals()[f] = cfg[f]
                del cfg
                ret = True
        else:
            pass

    if(not ret):
        print("no configuration file found - using defaults")
    return ret


def parse_args(argv) -> argparse.Namespace:
    global myscript, myapp, mydir
    
    read_config()
    parser=argparse.ArgumentParser(
        usage = "%(prog)s -p|--platform <platform> [OPTIONS] [PROJECT]",
        description = "%(prog)s: configure and build a 'CMAKE' based project in a cross compilation container"
    )
    parser.add_argument('project', nargs = '?', default = 'usc', help = 'project to build (default = \'usc2\')')

    pl = ['all'] + list(platforms.keys())
    parser.add_argument('-p', '--platform', action = 'extend', nargs = '+', required = True,
                        choices = pl, help = 'target platform, \'all\' or list of valid platforms')
    parser.add_argument('-C', '--container', nargs = '?', help = 'container to use (e.g. \'crosscompile\' or \'troja:443/crosscompile:latest\')')
    parser.add_argument('-a', '--app', nargs = '?', default = 'podman', help = 'define container manager [podman]')
    parser.add_argument('-t', '--target', nargs = '?', default = 'all', help = 'build target [all]')
    parser.add_argument('-s', '--source', nargs = '?', help = 'project source directory relative to project base platform (CMakeLists.txt)')
    parser.add_argument('-D', '--defines', nargs = '+', action = 'extend', help = 'additional defines (xxx=<value>)')
    parser.add_argument('-g', '--generator', nargs = '?', choices = generators.keys(), default = 'Make', help = 'Cmake generator to use')
    parser.add_argument('-l', '--lin', action = 'store_true', help = 'activate LIN bus support')
    parser.add_argument('-S', '--subsys', nargs = '?', choices = ['SRU', 'SSU'], default = 'SRU', help = 'define LIN sub system')
    parser.add_argument('-c', '--clean', action = 'store_true', help = 'clean before build')
    parser.add_argument('-i', '--interactive', action = 'store_true', help = 'start container in interactive mode')
    parser.add_argument('-q', '--qspy', action = 'store_true', help = 'enable support for QSPY')
    parser.add_argument('-v', '--verbose', action = 'store_true', help = 'verbose build')
    parser.add_argument('-d', '--debug', action = 'store_true', help = 'enable script debugging')
    parser.add_argument(
        "-V", "--version", action = "version",
        version = f"{parser.prog} version 1.0.0"
    )
    myscript = os.path.abspath(__file__)
    myapp = os.path.basename(myscript)
    mydir = os.path.dirname(myscript)

    args = parser.parse_args(argv)
    if('all' in args.platform):
        args.platform = list(platforms.keys())

    return args


def incontainer() -> bool:
    with open("/proc/1/sched", "r") as fp:
        a = fp.readline().split(' ')[0]
    if(a == 'systemd'):
        return False
    else:
        return True

def setupdatabase(args) -> dict:
    db = {
        'TRACE': None,
        'VERBOSE': None,
    }
    if(incontainer()):
        db['TC_BASE_DIR'] = '/cmake'
        db['PRJ_BASE_DIR'] = '/'
    else:
        db['TC_BASE_DIR'] = f"{os.environ['HOME']}/cmake"
        db['PRJ_BASE_DIR'] = f"{os.environ['HOME']}/Projects"
    
    db['CFG'] = globalcfg + [f"CONFIG_QSPY={str(args.qspy).upper()}", f"CONFIG_VERBOSE={str(args.verbose).upper()}"]

    if(args.verbose): db['TRACE'] = "--trace"
    if(args.verbose or args.clean): db['VERBOSE'] = "--clean-first"

    if(args.debug):
        print(f"{db=}")
    
    return db

def setvariables(platform, args, db) -> dict:
    vars = dict()
    Dprefix = "-D"

    vars['container'] = args.container
    vars['toolchainfile'] = f"-DCMAKE_TOOLCHAIN_FILE={db['TC_BASE_DIR']}/tc_{platforms[platform]['toolchain']}.cmake" if(platforms[platform]['toolchain']) else None
    vars['generator'] = args.generator if(args.generator) else platforms[platform]['generator']
    vars['prjdir'] = f"{db['PRJ_BASE_DIR']}/{args.project}"
    vars['srcdir'] = "/src" if(incontainer()) else f"{vars['prjdir']}/Source"
    vars['blddir'] = "/bld" if(incontainer()) else f"{vars['prjdir']}/Build"
    vars['containercmd'] = None if(args.interactive) else ["/usr/bin/python3", f"/work/{myapp}"]
    vars['interactive'] = ["-it"] if(args.interactive) else None
    
    l = db['CFG'] + platforms[platform]['extracfg']
    if(args.defines):
        l += args.defines
    vars['cfg'] = [Dprefix + c.strip() for c in l if(isinstance(c, str))]
    if(args.debug): print(f"All project vars:\n\t{vars=}")

    return vars

def buildargstring(args) -> list:
    argstring = []

    if(args.target): argstring.append(f'--target={args.target}')
    if(args.source): argstring.append(f'--source={args.source}')
    if(args.source): argstring.append(f'--source={args.source}')
    if(args.generator): argstring.append(f'--generator={args.generator}')
    if(args.defines):
        l = ["-D" + d.strip() for d in args.defines if(isinstance(d, str))]
        argstring += l
    if(args.lin):
        argstring += [f"--lin", f"--subsys={args.subsys}"]
    if(args.clean): argstring.append("--clean")
    if(args.interactive): argstring.append("--interactive")
    if(args.qspy): argstring.append("--qspy")
    if(args.verbose): argstring.append("--verbose")
    if(args.debug): argstring.append("--debug")
    argstring += [f"--", f"{args.project}"]

    return argstring


def startprocess(cmd, blddir) -> None:
    print(f"calling: {' '.join(cmd)}", flush=True)
    ret = subprocess.run(cmd, cwd = blddir, text = True) #, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
    print(f"return code = {ret.returncode}\n", flush=True) # {ret.stdout}\n", flush=True)
    if(ret.returncode != 0):
        sys.exit(f"subprocess {cmd[0:2]} exited unexpectedly: {ret.returncode}")
    else:
        print("\n", flush=True)


def build(pf, args, db) -> None:
    vars = setvariables(pf, args, db)
    gen = vars['generator']
    cgen = generators[gen]
    blddir = f"{vars['blddir']}/{pf}_{gen}"
    srcdir = vars['srcdir']

    intro = f""" Configuring and building project \"{args.project}\"
    target    = {args.target}
    platform  = {pf}
    generator = {gen}
    SRCDIR    = {srcdir}
    BLDDIR    = {blddir}
    CFG       = {' '.join(vars['cfg'])}
    """

    print(intro, flush=True)
    
    if(not os.path.exists(blddir)): os.makedirs(blddir)
    cmd = ['cmake']
    if(db['TRACE']): cmd += [db['TRACE']]
    cmd += ["-G", cgen, "-B."]
    if(vars['toolchainfile']): cmd += [vars['toolchainfile']]
    cmd += vars['cfg'] + [vars['srcdir']]
    startprocess(cmd, blddir)

    cmd = ['cmake', '--build', '.', '--target', args.target]
    if(db['VERBOSE']): cmd += [db['VERBOSE']]
    startprocess(cmd, blddir)


def startbuild(args) -> None:
    if(args.debug):
        print("startbuild()")
    
    db = setupdatabase(args)
    if(not args.interactive):
        for pf in args.platform:
            build(pf, args, db)


def startcontainer(pf, args, db) -> None:
    vars = setvariables(pf[0], args, db)
    argstring = ["--platform"] + pf + buildargstring(args)
    if(args.debug): print(f"{argstring=}")

    if(not os.path.exists(vars['blddir'])): os.makedirs(vars['blddir'])
    name = re.sub("^.*/([^/:]+).*$", r'\1', vars['container']) + '_' + str(os.getpid())
    cmd = [args.app, "run", "--rm", f"--name={name}", 
           f"-v{vars['srcdir']}:/src:ro", f"-v{vars['blddir']}:/bld", f"-v{vars['prjdir']}:/work", f"-v{db['TC_BASE_DIR']}:/cmake:ro", f"-v{myscript}:/work/{myapp}:ro",
           "-ePKG_CONFIG_PATH=/usr/lib/arm-linux-gnueabihf/pkgconfig", "-w/work"]
    if (vars['interactive']): cmd += vars['interactive']
    cmd += [vars['container']]
    if(vars['containercmd']): cmd += vars['containercmd'] + argstring
    startprocess(cmd, None)


def start(args) -> None:
    if(args.debug): print("start()")
    myplatforms = {}

    db = setupdatabase(args)
    if(args.container):
        startcontainer(args.platform, args, db)
    else:
        for cont in containers.keys():
            r = intersection(containers[cont], args.platform)
            if(r):
                myplatforms[cont] = r
        if(args.debug): print(f"{myplatforms=}")

        for cont in myplatforms.keys():
            if(cont == 'none'):
                for pf in myplatforms[cont]:
                    build(pf, args, db)
            else:
                args.container = cont
                db = setupdatabase(args)
                startcontainer(myplatforms[cont], args, db)


def main() -> None:
    args = parse_args(sys.argv[1:])
    if(args.debug):
        print(f"{args=}")

    if(incontainer()):
        if(args.debug):
            lcontainer = None
            pf = args.platform[0]
            for c in containers.keys():
                if(pf in containers[c]):
                    lcontainer = c
                    break
            print(f"running in container {args.container if (args.container) else lcontainer}")
        startbuild(args)
    else:
        start(args)

if (__name__ == "__main__"):
    main()
