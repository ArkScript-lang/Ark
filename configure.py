#!/usr/bin/env python3.6
import os
import sys
import urllib.request
import shutil
import tarfile
if os.name == 'nt':
    import zipfile, winreg, webbrowser, glob
import platform


def main():
    # download mpir
    print("Downloading mpir-3.0.0.tar.bz2...")
    if not os.path.exists("./mpir-3.0.0.tar.bz2"):
        url = "http://mpir.org/mpir-3.0.0.tar.bz2"
        with urllib.request.urlopen(url) as response, open("mpir-3.0.0.tar.bz2", 'wb') as out_file:
            shutil.copyfileobj(response, out_file)
    else:
        print("mpir-3.0.0.tar.bz2 found, not downloading it again. If the download failed, delete the file and retry")

    # extract all
    if not os.path.exists("./mpir-3.0.0/"):
        print("Extracting tarbal tp mpir-3.0.0/")
        with tarfile.open("mpir-3.0.0.tar.bz2", "r:bz2") as tar:
            tar.extractall("./")
    else:
        print("Tarbal already extracted, not doing it again. If the extraction failed, delete the folder an retry")
    
    if os.name == 'posix':
        print("Installing yasm...")
        os.system("apt-get install yasm -y -q")

        # install it on the system
        print("Configuring, building and installing mpir...")
        os.system("cd ./mpir-3.0.0/ && ./configure --enable-cxx && make && make check && make install")
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
        
        if not os.path.exists("./yasm/vsyasm.exe"):
            print(f"Please download yasm (arch: {arch}) and put it into the yasm folder generated...")
            webbrowser.open(url)
            
            if not os.path.exists("yasm/"):
                os.mkdir("yasm/")
            
            print("Press Enter when you're done...")
            input()
        
        msys = lambda x: os.system(f"cd {os.getcwd()} && C:\\MinGW\\msys\\1.0\\bin\\bash.exe -c \"{x}\"")
        
        if not os.path.exists("C:\\MinGW\\msys\\1.0\\bin\\bash.exe"):
            print("This script needs msys, please install it and retry")
            sys.exit(1)
        
        # configure project
        print("Configuring mpir...")
        msys("cd ./mpir-3.0.0/ && ./configure --disable-static --enable-shared --with-yasm=../yasm/vsyasm.exe")
        
        # find visual studio version (must be ran as admin)
        print("Searching for installed version of Visual Studio...")
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
        print(f"Found version: {vs_ver}")

        # build from command line
        ms_arch = "win32" if arch == 32 else "x64"
        toolset = {"15": "v141", "14": "v140", "12": "v120", "11": "v110"}[vs_ver]
        content = ""
        line = "%msbdir%\msbuild.exe /p:Platform=%plat% /p:Configuration=%conf% %srcdir%\%src%\%src%.vcxproj"
        new_line = f"{line} /p:PlatformToolset={toolset}"
        print("Reconfiguring build script...")
        with open(f"mpir-3.0.0/build.vc{vs_ver}/msbuild.bat") as f:
            content = f.read()
        if new_line not in content:
            content = content.replace(line, new_line)
        with open(f"mpir-3.0.0/build.vc{vs_ver}/msbuild.bat", "w") as f:
            f.write(content)
        
        print("Please open mpir.sln, right click on solution and retarget it...")
        os.system(f"cd mpir-3.0.0\\build.vc{vs_ver} && explorer .")
        print("Press Enter when you're done...")
        input()

        print(f"Building mpir (arch: {ms_arch})...")
        os.system(f"cd mpir-3.0.0\\build.vc{vs_ver} && msbuild.bat gc dll {ms_arch} Release")
        os.system(f"cd mpir-3.0.0\\build.vc{vs_ver} && msbuild.bat cxx lib {ms_arch} Release")

        print("Moving files...")
        subdir = glob.glob("mpir-3.0.0/dll/*/")[0]
        for f in glob.glob(f"{subdir}/Release/*.*"):
            try:
                os.rename(f, os.path.dirname(f) + "/../../" + os.path.basename(f))
            except FileExistsError:
                pass
        shutil.rmtree(subdir, ignore_errors=True)

        subdir = glob.glob("mpir-3.0.0/lib/*/")[0]
        for f in glob.glob(f"{subdir}/Release/*.*"):
            try:
                os.rename(f, os.path.dirname(f) + "/../../" + os.path.basename(f))
            except FileExistsError:
                pass
        shutil.rmtree(subdir, ignore_errors=True)
    
    print("Done!")
    sys.exit(0)


if __name__ == '__main__':
    main()