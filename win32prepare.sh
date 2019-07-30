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

if [ ${WINDOWS} = "1" ]; then
    echo check_required_package mingw-w64
    echo check_required_package mingw-w64-dev
    echo check_required_package mingw-w64-tools
fi

# download and unpack
if [ ! -d ${PWD_DIR}/vlc-${VLC_VERSION} ]; then
    cd ${PWD_DIR}
    
    if [ ! -f ${PWD_DIR}/vlc-${VLC_VERSION}.tar.* ]; then
        info "Downloading vlc"       
        if [ ${VLC_VERSION} = "2.2"  ]; then
           git clone https://git.videolan.org/git/vlc/vlc-2.2.git
        else
           download ${VLC_URL} || error "Failed to download vlc"
           unpack ${PWD_DIR}/vlc-${VLC_VERSION}.tar.*
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
           if [ "$i" -eq "11" ]; then 
              check_and_patch ${PWD_DIR}/patches/${VLC_VERSION}/11-makefiles.patch
           elif  [ "$i" -eq "12" ]; then
              check_and_patch ${PWD_DIR}/patches/${VLC_VERSION}/12-root.patch
           elif  [ "$i" -eq "14" ]; then
              check_and_patch ${PWD_DIR}/patches/${VLC_VERSION}/14-extras.patch
           elif  [ "$i" -eq "29" ]; then
              check_and_patch ${PWD_DIR}/patches/${VLC_VERSION}/29-configure-ac.patch
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
apply_patch ${PWD_DIR}/patches/qt4/0058-qt4-remove-debug-assert-in-inputmanager.patch

cd ${PWD_DIR}

### now skip few patches
if [ ${WINDOWS} = "1" ]; then
    check_and_patch ${PWD_DIR}/patches/win32/0000-win32-vlc_win32_rc.patch
    check_and_patch ${PWD_DIR}/patches/win32/0001-win32-remove-npapi.patch
    check_and_patch ${PWD_DIR}/patches/win32/0002-win32-remove-npapi.patch
    check_and_patch ${PWD_DIR}/patches/win32/0003-win32-fix-tsplayer-name.patch
    echo check_and_patch ${PWD_DIR}/patches/win32/0004-win32-qt4-main_interface.patch
    check_and_patch ${PWD_DIR}/patches/win32/0005-win32-winvlc-devkey.patch 
    check_and_patch ${PWD_DIR}/patches/win32/0011-win32-qt4-main_interface.patch   
	check_and_patch ${PWD_DIR}/patches/win32/0012-win32-qt4.patch
    check_and_patch ${PWD_DIR}/patches/win32/0013-win32-opensll.patch
    check_and_patch ${PWD_DIR}/patches/win32/0014-win32-sqlite.patch
    check_and_patch ${PWD_DIR}/patches/win32/0015-win32-qt-4.8.6.patch
    echo check_and_patch ${PWD_DIR}/patches/win32/0016-win32-qt4-before-phtreads.patch
    if [ ${QT_VERSION} = "4.8.5" ]; then
        check_and_patch ${PWD_DIR}/patches/win32/0017-comeback-to-qt-4.8.5.patch
    fi
    check_and_patch ${PWD_DIR}/patches/win32/0018-win32-compile-libqjpeg-before-libjpeg.patch
fi

# private directory
if [ -d  ${PWD_DIR}/private/vlc ]; then
    cp -rf ${PWD_DIR}/private/vlc/* ${PWD_DIR}/vlc-${VLC_VERSION}
fi

info "Ok"

# bootstraping vlc
${PWD_DIR}/vlc-${VLC_VERSION}/bootstrap

# win32 Makefile patch
cd ${PWD_DIR}
check_and_patch ${PWD_DIR}/patches/win32/1000-win32-fix-Makefile-after-bootstrap.patch

info "Ok"
