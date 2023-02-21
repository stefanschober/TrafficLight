import argparse, subprocess, json, sys, os, re
from pathlib import Path
from shutil import rmtree

myscript = None
myapp = None
mydir = None

def parse_args(argv) -> argparse.Namespace:
    global myscript, myapp, mydir

    parser=argparse.ArgumentParser(
        usage = "%(prog)s [OPTIONS] <--file/-f [FILE]> (default: compile_commands.json)",
        description = "%(prog)s: patch the CMAKE generated file compile_commands.json to hold all project include paths"
    )
    parser.add_argument('--file', '-f', nargs = '?', default = 'compile_commands.json', help = "file to process")

    myscript = str(Path(__file__).resolve())
    myapp = str(Path(myscript).name)
    mydir = str(Path(myscript).parent)

    args = parser.parse_args(argv)
    print(f"***** {parser.prog} *****")

    if not args.file:
        parser.print_usage()
        sys.exit(-1)

    return args

def patch(cc: dict) -> bool:
    i = 1
    regex = '@[^@\s]+[@\s]'

    for c in cc:
        # print(f"{i}:\tdirectory: {c['directory']}\n\tfile: {c['file']}\n\tcommand: {c['command']}\n")
        # i = i+1
        m = re.search(regex, c['command'])
        if m:
            p = Path(m.group()[1:])
            if p.exists() and p.is_file():
                s = ""
                try:
                    with p.open('r') as fp:
                        for x in fp:
                            s = s + x
                except:
                    return False
                c['command'] = re.sub(regex, s, c['command']).replace('\n', ' ')

    return True

def main() -> int:
    compile_commands = dict()

    args = parse_args(sys.argv[1:])
    p = Path(args.file)
    if(p.exists() and p.is_file()):
        print(f"patching {p}...")
        try:
            with p.open("r") as fp:
                compile_commands = json.load(fp)
        except:
            return -1
    
    if patch(compile_commands):
        print("          ... done!")
        try:
            with p.open("w") as fp:
                json.dump(compile_commands, fp, indent=4)
        except:
            return -1

    return 0

 
if (__name__ == "__main__"):
    sys.exit(main())
