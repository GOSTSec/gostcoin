# Building wallet with MSYS2 MinGW under Windows
*All actions taken under 32-bit (mingw32) shell*

Replace `<treads>` to number of threads (cores) used to build.

### Preparing to build
```bash
$ pacman -S diff git make tar mingw-w64-x86_64-gcc mingw-w64-x86_64-boost mingw-w64-x86_64-libpng mingw-w64-x86_64-openssl mingw-w64-x86_64-zlib mingw-w64-x86_64-db
$ mkdir /c/devel && cd /c/devel
```
#### Caveat
If you are an Arch Linux user, refrain from updating system with `pacman -Syu`.
Always update runtime separately as described on the home page,
otherwise you might end up with DLLs incompatibility problems.
#### Building QREncode
```bash
$ cd ../..
$ wget https://fukuchi.org/works/qrencode/qrencode-4.0.0.tar.gz
$ tar xzf qrencode-4.0.0.tar.gz
$ cd qrencode-4.0.0
$ ./configure --enable-static --prefix=/mingw32
$ make -j <threads> && make install
```
#### Cloning repository
```bash
$ cd ..
$ git clone https://github.com/GOSTSec/gostcoin.git
$ cd gostcoin
```

### Building wallet daemon
```bash
$ cd src
$ make -f makefile.mingw -j <threads>
```
### Building QT GUI wallet
```bash
$ pacman -S mingw-w64-x86_64-qt5-base  mingw-w64-x86_64-qt5-tools
$ qmake
$ make -j <threads>
```

### Building Qt 5.9 Opensource staticly
**[!] Requires Python 2.7**

[Download](https://www.python.org/downloads/release/python-2714/) and install Python, for example, to `C:\devel\Python27` and add it to PATH
```bash
$ export PATH="$PATH:/c/devel/Python27"
```

### Building QT
```bash
$ wget http://download.qt.io/official_releases/qt/5.9/5.9.2/single/qt-everywhere-opensource-src-5.9.2.zip
$ unzip qt-everywhere-opensource-src-5.9.2.zip
$ cd qt-everywhere-opensource-src-5.9.2
$ CXXFLAGS="--Wno-deprecated-declarations" ./configure -static -release -opensource -confirm-license -platform win32-g++ -prefix "C:\devel\qt5.9.2-static" -nomake examples -nomake tests -nomake tools -opengl desktop -no-angle -make libs -qt-zlib -qt-libpng -qt-libjpeg -qt-freetype -qt-pcre -sql-sqlite
$ make -j <threads> && make install
```

### Build wallet with QT
```bash
$ export PATH="$PATH:/c/devel/qt5.9.2-static/bin"
$ qmake RELEASE=1 USE_QRCODE=1 USE_IPV6=1 USE_BUILD_INFO=1
$ make -j <threads>
```

**Copyright © 2018 R4SAS <r4sas@i2pmail.org>**
