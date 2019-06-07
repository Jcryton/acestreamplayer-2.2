#!/bin/bash

echo Fix Acestream files "/usr/bin/acestreamplayer,/usr/bin/acestreamplayer-wrapper,/usr/lib64/pkgconfig/libacestreamplayer.pc,/usr/lib64/pkgconfig/acestreamplayer-plugin.pc"
cd /usr/bin
mv vlc acestreamplayer 2>/dev/null
mv vlc-wrapper acestreamplayer-wrapper 2>/dev/null
cd /usr/lib64/pkgconfig
mv libvlc.pc acestreamplayervlc.pc 2>/dev/null
mv vlc-plugin.pc acestreamplayer-plugin.pc 2>/dev/null

echo Restore VLC files "/usr/bin/vlc,/usr/bin/vlc-wrapper,/usr/lib64/pkgconfig/libvlc.pc,/usr/lib64/pkgconfig/vlc-plugin.pc"
cd /usr/bin
mv _vlc vlc 2>/dev/null
mv _vlc-wrapper vlc-wrapper 2>/dev/null
cd /usr/lib64/pkgconfig
mv _libvlc.pc libvlc.pc 2>/dev/null
mv _vlc-plugin.pc vlc-plugin.pc 2>/dev/null


