import os, sys, time, re

def add_subpaths(path, rule):
    paths = []
    print(rule)
    for p in os.listdir(path):
        subpath = os.path.join(path, p)
        if os.path.isdir(subpath): 
            paths.extend(add_subpaths(subpath, rule))
            continue
        if rule == "":
            paths.append(subpath)
        else:
            if len(re.findall(rule, p)) > 0:
                paths.append(subpath)
    return paths

def gather_files(tracked_files_paths):
    files = set([])
    file_rules = []
    for fi in tracked_files_paths:
        fi = fi.replace("\\", "\\\\")
        fi = fi.replace(".", "\\.")
        fi = fi.replace("*", ".*")
        fi = "^"+fi+"$"
        file_rules.append(fi)

    rule = "|".join(file_rules)
    for fi in file_rules:
        fls = []
        if rule == "":
            fls = [f for f in os.listdir(os.getcwd())]
        else:
            fls = [f for f in os.listdir(os.getcwd()) if len(re.findall(fi, f)) > 0]
        dirs = [f for f in fls if os.path.isdir(f)]
        fls = [f for f in fls if not os.path.isdir(f)]
        [fls.extend(add_subpaths(d, rule)) for d in dirs]
        [files.add(f) for f in fls]
    return files

def timeDict(list):
    return dict([(f, os.path.getmtime(f)) for f in list])
    
def main():
    if len(sys.argv) < 2:
        print("please specify a command to execute on change and path(s) to files to be monitored")
        return
    if len(sys.argv) < 3:
        print("please specify at least one file to be monitored")
        return
    command = sys.argv[1]
    tracked_files = timeDict(gather_files(sys.argv[2:]))
    print("files to be monitored:")
    for f in tracked_files.keys(): print(f)
    print("command to execute: " + command)

    i=1
    while(True):
        new_files = gather_files(sys.argv[2:])
        added_files = set(new_files).symmetric_difference(tracked_files)
        if len(added_files)>0:
            print("files added/removed:")
            for f in added_files:
                if f in tracked_files:
                    tracked_files.pop(f)
                    print("- "+f)
                else:
                    tracked_files[f] = os.path.getmtime(f)
                    print("+ "+f)
        update = False
        for f, t in tracked_files.items():
            newtime = os.path.getmtime(f)
            if newtime > t:
                update = True
                tracked_files[f] = newtime
        if update:
            sys.stdout.write("executing command ... ")
            os.system(command)
            sys.stdout.write("done\n")
        time.sleep(2)

if __name__ == "__main__":
    main()
