#!/bin/sh

export QT_SELECT=qt4

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

if [ ${WINDOWS} = "1" ]; then
    check_required_package mingw-w64
    check_required_package mingw-w64-dev
    check_required_package mingw-w64-tools
fi

# download and unpack
if [ ! -d ${PWD_DIR}/vlc-${VLC_VERSION} ]; then
    cd ${PWD_DIR}
    
    if [ ! -f ${PWD_DIR}/vlc-${VLC_VERSION}.tar.* ]; then
        info "Downloading vlc"       
        if [ ${VLC_VERSION} != "2.1"  ]; then
           download ${VLC_URL} || error "Failed to download vlc"
           unpack ${PWD_DIR}/vlc-${VLC_VERSION}.tar.*
        else
           git clone https://git.videolan.org/git/vlc/vlc-2.2.git
        fi

    fi
    #unpack ${PWD_DIR}/vlc-${VLC_VERSION}.tar.*
fi

if [ ! -f ${PWD_DIR}/._prepare ]; then
    # copy new sources
    rm -r ${PWD_DIR}/vlc-${VLC_VERSION}/share/icons || error ""
    rm ${PWD_DIR}/vlc-${VLC_VERSION}/share/vlc512x512.png || error ""

    cp -rf ${PWD_DIR}/acestreamplayer/include/* ${PWD_DIR}/vlc-${VLC_VERSION}/include || error ""
    cp -rf ${PWD_DIR}/acestreamplayer/lib/* ${PWD_DIR}/vlc-${VLC_VERSION}/lib || error ""
    cp -rf ${PWD_DIR}/acestreamplayer/modules/* ${PWD_DIR}/vlc-${VLC_VERSION}/modules || error ""
    cp -rf ${PWD_DIR}/acestreamplayer/share/* ${PWD_DIR}/vlc-${VLC_VERSION}/share || error ""
    cp -rf ${PWD_DIR}/acestreamplayer/src/* ${PWD_DIR}/vlc-${VLC_VERSION}/src || error ""

    # preparing for patching
    mv ${PWD_DIR}/vlc-${VLC_VERSION}/share/vlc.desktop.in ${PWD_DIR}/vlc-${VLC_VERSION}/share/acestreamplayer.desktop.in || error ""

    mv ${PWD_DIR}/vlc-${VLC_VERSION}/share/solid/vlc-openvcd.desktop ${PWD_DIR}/vlc-${VLC_VERSION}/share/solid/acestreamplayer-openvcd.desktop || error ""
    mv ${PWD_DIR}/vlc-${VLC_VERSION}/share/solid/vlc-opendvd.desktop ${PWD_DIR}/vlc-${VLC_VERSION}/share/solid/acestreamplayer-opendvd.desktop || error ""
    mv ${PWD_DIR}/vlc-${VLC_VERSION}/share/solid/vlc-openbd.desktop ${PWD_DIR}/vlc-${VLC_VERSION}/share/solid/acestreamplayer-openbd.desktop || error ""
    mv ${PWD_DIR}/vlc-${VLC_VERSION}/share/solid/vlc-opencda.desktop ${PWD_DIR}/vlc-${VLC_VERSION}/share/solid/acestreamplayer-opencda.desktop || error ""

    mv ${PWD_DIR}/vlc-${VLC_VERSION}/share/utils/video-vlc-default.sh ${PWD_DIR}/vlc-${VLC_VERSION}/share/utils/video-acestreamplayer-default.sh || error ""
    mv ${PWD_DIR}/vlc-${VLC_VERSION}/share/utils/audio-vlc-default.sh ${PWD_DIR}/vlc-${VLC_VERSION}/share/utils/audio-acestreamplayer-default.sh || error ""
    mv ${PWD_DIR}/vlc-${VLC_VERSION}/share/utils/gnome-vlc-default.sh ${PWD_DIR}/vlc-${VLC_VERSION}/share/utils/gnome-acestreamplayer-default.sh || error ""

    mv ${PWD_DIR}/vlc-${VLC_VERSION}/po/vlc.pot ${PWD_DIR}/vlc-${VLC_VERSION}/po/acestreamplayer.pot || error ""
    touch ${PWD_DIR}/._prepare
fi

# patch
check_and_patch()
{
    cd ${PWD_DIR}/vlc-${VLC_VERSION}
    
    filename=$(basename "$1")
    name="${filename%.*}"
    if [ ! -f ${PWD_DIR}/._${name} ]; then
        apply_patch $1
        touch ${PWD_DIR}/._${name}
    fi
    
    cd ${PWD_DIR}
}

# revert plmodel patch
check_and_patch ${PWD_DIR}/patches/plmodel/0001-Revert-playlist-model.patch

# common patches
for i in `seq 1 29`; do
    if [ "$i" -eq "20" ]; then 
       for i in `seq 1 47`; do
          if [ "$i" -ne "39" ]; then
               if [ "$i" -lt "10" ]; then
                   check_and_patch ${PWD_DIR}/patches/qt4/000$i-qt4-modules.patch
               else
                   check_and_patch ${PWD_DIR}/patches/qt4/00$i-qt4-modules.patch
               fi
          fi 
       done   
    else
       if [ "$i" -lt "10" ]; then 
           check_and_patch ${PWD_DIR}/patches/common/0$i-*.patch
       else
           if [ "$i" -eq "12" ]; then 
              check_and_patch ${PWD_DIR}/patches/${VLC_VERSION}/12-root.patch
           else  
              check_and_patch ${PWD_DIR}/patches/common/$i-*.patch
           fi
       fi
    fi
done

# update... fix.. it
cd ${PWD_DIR}/vlc-${VLC_VERSION}

apply_patch ${PWD_DIR}/patches/qt4/0048-qt4-opendialog.patch
apply_patch ${PWD_DIR}/patches/qt4/0050-qt4-recents.patch
apply_patch ${PWD_DIR}/patches/qt4/0051-qt4-dialogsprovider.patch
apply_patch ${PWD_DIR}/patches/qt4/0052-qt4-maininterface.patch
apply_patch ${PWD_DIR}/patches/qt4/0053-qt4-makefile.patch
apply_patch ${PWD_DIR}/patches/qt4/0054-qt4-dialogsprovider.patch
apply_patch ${PWD_DIR}/patches/qt4/0055-qt4-inputslider-1.patch
apply_patch ${PWD_DIR}/patches/qt4/0056-qt4-inputslider-2.patch
apply_patch ${PWD_DIR}/patches/qt4/0057-qt4-timetooltip.patch

# gentoo patch

if [ ${VLC_VERSION} == "2.2.4"  ]; then
apply_patch ${PWD_DIR}/patches/gentoo/vlc-2.1.0-fix-libtremor-libs.patch
apply_patch ${PWD_DIR}/patches/gentoo/vlc-9999-libva-1.2.1-compat.patch
apply_patch ${PWD_DIR}/patches/gentoo/vlc-2.1.0-TomWij-bisected-PA-broken-underflow.patch
apply_patch ${PWD_DIR}/patches/gentoo/vlc-2.2.4-relax_ffmpeg.patch
apply_patch ${PWD_DIR}/patches/gentoo/vlc-2.2.4-ffmpeg3.patch
apply_patch ${PWD_DIR}/patches/gentoo/vlc-2.2.4-decoder-lock-scope.patch
apply_patch ${PWD_DIR}/patches/gentoo/vlc-2.2.4-alsa-large-buffers.patch
apply_patch ${PWD_DIR}/patches/gentoo/vlc-2.2.4-libav-11.7.patch
fi

cd ${PWD_DIR}

if [ ${WINDOWS} = "1" ]; then
    # prebuilt contribs
    if [ ! -d ${PWD_DIR}/vlc-${VLC_VERSION}/contrib/${HOST} ]; then
        cd ${PWD_DIR}/vlc-${VLC_VERSION}/contrib
        
        if [ ! -f ${PWD_DIR}/vlc-${VLC_VERSION}/contrib/${HOST}.tar.* ]; then
            info "Downloading contribs"
            # TODO
            download ${CONTRIBS_URL} || error "Failed to download contribs"
        fi
        
        unpack ${PWD_DIR}/vlc-${VLC_VERSION}/contrib/${HOST}.tar.*
        cd ${PWD_DIR}/vlc-${VLC_VERSION}/contrib/${HOST}
        ./change_prefix.sh
        
        cd ${PWD_DIR}
    fi
fi

# private directory
if [ -d  ${PWD_DIR}/private/vlc ]; then
    cp -rf ${PWD_DIR}/private/vlc/* ${PWD_DIR}/vlc-${VLC_VERSION}
fi

# bootstraping vlc
${PWD_DIR}/vlc-${VLC_VERSION}/bootstrap

info "Ok"
