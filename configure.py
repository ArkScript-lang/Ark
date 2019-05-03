#!/usr/bin/env python3.6
import os
import sys
import urllib.request
import shutil
import tarfile
if os.name == 'nt':
    import zipfile, winreg
import platform


def main():
    # download mpir
    url = "http://mpir.org/mpir-3.0.0.tar.bz2"
    with urllib.request.urlopen(url) as response, open("mpir-3.0.0.tar.bz2", 'wb') as out_file:
        shutil.copyfileobj(response, out_file)
    
    # create a folder for it
    if not os.path.exists("./mpir-3.0.0/"):
        os.mkdir("./mpir-3.0.0")

    # extract all
    with tarfile.open("mpir-3.0.0.tar.bz2", "r:bz2") as tar:
        tar.extractall("./mpir-3.0.0/")
    
    if os.name == 'posix':
        # install it on the system
        os.system("cd ./mpir-3.0.0/ && ./configure && make && make check && make install")
    elif os.name == 'nt':
        # download yasm
        url = "http://tortall.net/projects/yasm/releases/vsyasm-1.3.0-win"

        arch = None
        if platform.architecture()[0] == '32bit':
            arch = 32
        elif platform.architecture()[0] == '64bit':
            arch = 64
        else:
            print("Can not detect architecture properly")
            sys.exit(1)
        
        url += f"{arch}.zip"
        
        with urllib.request.urlopen(url) as response, open("vsyasm-1.3.0.zip", 'wb') as out_file:
            shutil.copyfileobj(response, out_file)
        
        if not os.path.exists("C:/Program Files/yasm/"):
            os.mkdir("C:/Program Files/yasm/")
        
        # install yasm (must be run as admin)
        with zipfile.ZipFile("vsyasm-1.3.0.zip") as myzip:
            myzip.extractall("C:/Program Files/yasm/")
        
        # configure project
        os.system("cd ./mpir-3.0.0/ && ./configure --disable-static --enable-shared --with-yasm=C:/Program\\ Files/yasm/vsyasm.exe")
        
        # find visual studio version (must be ran as admin)
        key = "SOFTWARE\\Microsoft\\VisualStudio\\{version}"
        possible_versions = ["11.0", "12.0", "14.0", "15.0"]
        installed = []
        for ver in possible_versions:
            try:
                winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, key.format(version=ver), 0, winreg.KEY_ALL_ACCESS)
                installed.append(ver)
            except FileNotFoundError:
                pass
            except PermissionError:
                print("This script must be ran as admin to detect the visual studio version installed!")
                sys.exit(1)
        if installed == []:
            print("No version of Visual Studio found. Please install one and retry")
            sys.exit(1)
        vs_ver = installed[-1][:2]

        # build from command line
        ms_arch = "Win32" if arch == 32 else "x64"
        os.system(f"cd ./mpir-3.0.0/build.vc{vs_ver}/ && msbuild.bat gc dll {ms_arch} Release")


if __name__ == '__main__':
    main()