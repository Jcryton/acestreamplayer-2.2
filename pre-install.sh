#!/bin/bash

echo Backup VLC files "/usr/bin/vlc,/usr/bin/vlc-wrapper,/usr/lib64/pkgconfig/libvlc.pc,/usr/lib64/pkgconfig/vlc-plugin.pc"
cd /usr/bin
mv vlc _vlc 2>/dev/null
mv vlc-wrapper _vlc-wrapper 2>/dev/null
cd /usr/lib64/pkgconfig
mv libvlc.pc _libvlc.pc 2>/dev/null
mv vlc-plugin.pc _vlc-plugin.pc 2>/dev/null


