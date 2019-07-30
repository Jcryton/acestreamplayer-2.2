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
	echo "####################################################"
	echo "### RUN: win32prepare.sh before win32tarballs.sh ###"
	echo "####################################################"
    exit
fi

# change into vlc dir
cd ${PWD_DIR}/vlc-${VLC_VERSION}
mkdir -p contrib/tarballs && cd contrib/tarballs


# download tarballs

wget http://master.qt.io/archive/qt/4.8/${QT_VERSION}/qt-everywhere-opensource-src-${QT_VERSION}.tar.gz
mv qt-everywhere-opensource-src-${QT_VERSION}.tar.gz qt-${QT_VERSION}.tar.gz

wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/a52dec-0.7.4.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/chromaprint-1.0.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/crystalhd_lgpl_includes_v1.zip
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/d3d11.idl
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/d3d11.idl.orig
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/dxgidebug.idl
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/faad2-2.7.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/ffmpeg-cdb0225.tar.xz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/flac-1.3.1.tar.xz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/fontconfig-2.11.1.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/freetype-2.6.2.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/fribidi-0.19.7.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/game-music-emu-0.6.0.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/gettext-0.19.6.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/glew-1.7.0.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/gmp-6.0.0.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/gnutls-3.2.21.tar.xz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/goom-2k4-0-src.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/harfbuzz-1.0.6.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/jpegsrc.v9a.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/lame-3.99.5.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libass-0.13.2.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libbluray-0.9.3.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libcaca-0.99.beta17.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libcddb-1.3.2.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libdca-0.0.5.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libdvbpsi-1.3.1.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libdvdcss-1.4.0.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libdvdnav-5.0.3.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libdvdread-5.0.3.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libebml-1.3.3.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libgcrypt-1.6.4.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libgpg-error-1.20.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libgsm_1.0.13.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libiconv-1.14.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libkate-0.4.1.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libmad-0.15.1b.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libmatroska-1.4.4.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libmodplug-0.8.8.5.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libmpeg2-0.5.1.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libogg-1.3.2.tar.xz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libpng-1.6.19.tar.xz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libsamplerate-0.1.8.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libshout-2.4.1.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libssh2-1.4.3.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libtheora-1.1.1.tar.xz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libupnp-1.6.19.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/LibVNCServer-0.9.9.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libvorbis-1.3.5.tar.xz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libvpx-1.4.0.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/libxml2-2.9.3.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/live.2016.02.22.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/lua-5.1.4.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/musepack_src_r481.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/nettle-2.7.1.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/oaidl.idl
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/openjpeg-1.5.0.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/openssl-1.0.1e.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/opus-1.1.1.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/orc-0.4.18.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/postproc-git.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/projectM-2.0.1-Source.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/pthreads-w32-2-9-1-release.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/regex-0.13.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/schroedinger-1.0.11.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/SDL-1.2.15.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/SDL_image-1.2.12.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/sidplay-libs-2.1.1.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/speex-1.2rc2.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/speexdsp-1.2rc3.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/sqlite-3.6.20.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/taglib-1.11.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/tiff-4.0.3.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/twolame-0.3.13.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/x264-git.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/x265-1.3.tar.bz2
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/zlib-1.2.8.tar.gz
wget https://raw.githubusercontent.com/Jcryton/ace-contrib/master/2.2/tarballs/zvbi-0.2.35.tar.bz2
