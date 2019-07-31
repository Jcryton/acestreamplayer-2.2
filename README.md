Ace Stream Player
-----------------

![](https://o9.icdn.ru/j/jcryton/1/64283811AmG.jpg)

Tested with VLC *v2.2.4 v2.2.5.1 v2.2.6 v2.2.7 v2.2.8 v2.2.8-git*

Depends: acestreamengine,
Optimal: acestreamengine-3.1.35

**Compile for Linux:** 

Install build tools *libtool*, *build-essential*, *pkg-config*, *autoconf*

Checkout all required [libraries] to build vlc

Also install *Qt4Core Qt4Gui Qt4WebKit >= 4.8 (optimal 4.8.6)*, *openssl*, *sqlite3*

Edit config.sh, set VLC_VERSION="2.2.4", "2.2.5.1", "2.2.6", "2.2.7", "2.2.8"
to build this release, or "2.2" to build with last vlc-2.2 git version

`./bootstrap.sh`

`./configure.sh`

`cd build-ace`

`make`

`su`

`make install`


**Crosscompile for Windows:**

Download [ubuntu-mate-16.04-desktop-amd64.iso], and instal on VirtualBox


***Ubuntu 16.04:***

`apt-get install -y \`

`gcc-mingw-w64-i686 g++-mingw-w64-i686 mingw-w64-tools \`

`mingw-w64-i686-dev mingw-w64-common build-essential \`

`wine64-development-tools libwine-development libwine-development-dev \`

`qt4-dev-tools qt4-default git subversion make gettext \`

`cmake cvs zip p7zip nsis bzip2 lua5.2 libtool pkg-config \`

`yasm cvs cmake ragel autopoint automake autoconf \`

`ant default-jdk protobuf-compiler \`

`dos2unix`

`git clone https://github.com/Jcryton/acestreamplayer-2.2.git`

`cd acestreamplayer-2.2`

edit config.sh (set ubuntu qt contrib and vlc version)

`./win32prepare.sh`

`./win32build.sh`

***Manual build contrib:***

`./win32prepare.sh`

`./win32tarballs.sh`

`./win32compile.sh`

[libraries]:https://wiki.videolan.org/Contrib_Status/
[ubuntu-mate-16.04-desktop-amd64.iso]:https://mirror.yandex.ru/ubuntu-cdimage/ubuntu-mate/releases/16.04/release/ubuntu-mate-16.04-desktop-amd64.iso
