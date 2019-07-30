#!/bin/sh

PWD_DIR=$(readlink -f $(dirname $0))
. "${PWD_DIR}/config.sh"
. "${PWD_DIR}/functions.sh"

LINUX="0"
WINDOWS="0"
case "${HOST}" in
  *mingw32*)
    WINDOWS="1"
  ;;
  *)
    LINUX="1"
  ;;
esac

if [ ${WINDOWS} = "0" ]; then
    echo "For compile AcestreamPlayer for linux"
    echo "Use sripts:"
    echo "bootstrap.sh configure.sh"
    exit
fi


if [ ! -f ${PWD_DIR}/._prepare ]; then
    echo "#################################################"
    echo "### RUN: win32prepare.sh and win32tarballs.sh ###"
    echo "#################################################"
    exit
fi

# change into vlc dir
cd ${PWD_DIR}/vlc-${VLC_VERSION}

# create and change into contrib dir
mkdir -p contrib/win32 && cd contrib/win32

#run the bootstrap
../bootstrap --host=i686-w64-mingw32

echo "##################################"
echo "### steps after build contrib: ###"
echo "##################################"
echo " "
echo "### remove the 64 bit binaries"
echo "rm -f ../i686-w64-mingw32/bin/moc ../i686-w64-mingw32/bin/uic ../i686-w64-mingw32/bin/rcc"
echo " "
echo "### go back and run the bootstrap"
echo "cd ${PWD_DIR}/vlc-${VLC_VERSION}"
echo "./bootstrap"
echo " "
echo "### create the dir we'll be compiling in"
echo "mkdir win32 && cd win32"
echo " "
echo "### tell the system where to find the pkgconfig dir"
echo "export PKG_CONFIG_LIBDIR=${PWD_DIR}/vlc-${VLC_VERSION}/contrib/i686-w64-mingw32/lib/pkgconfig"
echo " "
echo "### run configure"
echo "../extras/package/win32/configure.sh --host=i686-w64-mingw32 --enable-sqlite --disable-bluray"
echo " "
echo "### compile"
echo "make"

read -p "Press key to continue.. " -n1 -s

echo "build contrib"
make

echo "### remove the 64 bit binaries"
rm -f ../i686-w64-mingw32/bin/moc ../i686-w64-mingw32/bin/uic ../i686-w64-mingw32/bin/rcc

echo "### go back and run the bootstrap"
cd ${PWD_DIR}/vlc-${VLC_VERSION}
./bootstrap

echo "### create the dir we'll be compiling in"
mkdir win32 && cd win32

echo "### tell the system where to find the pkgconfig dir"
export PKG_CONFIG_LIBDIR=${PWD_DIR}/vlc-${VLC_VERSION}/contrib/i686-w64-mingw32/lib/pkgconfig

### run configure
../extras/package/win32/configure.sh --host=i686-w64-mingw32 --enable-sqlite --disable-bluray

# win32 Makefile patch
cd ${PWD_DIR}/vlc-${VLC_VERSION}
apply_patch ${PWD_DIR}/patches/win32/1000-win32-fix-Makefile-after-bootstrap.patch
apply_patch ${PWD_DIR}/patches/win32/1001-win32-fix-Makefile-after-configure.patch

echo "### compile"
cd ${PWD_DIR}/vlc-${VLC_VERSION}/win32
make

echo "build package"
make package-win32-zip


### copy archive to PWD dir
if [ ${VLC_VERSION} = "2.2"  ]; then
   cp ${PWD_DIR}/vlc-${VLC_VERSION}/win32/vlc-2.2.8.1-win32.zip ${PWD_DIR}/acestreamplyaer-2.2.8.1-win32.zip 
else
   cp ${PWD_DIR}/vlc-${VLC_VERSION}/win32/vlc-${VLC_VERSION}-win32.zip ${PWD_DIR}/acestreamplyaer-${VLC_VERSION}-win32.zip 
fi

echo "#################################################################"
echo "### Finally: acestreamplyaer-${VLC_VERSION}-win32.zip is ready###"
echo "#################################################################"
