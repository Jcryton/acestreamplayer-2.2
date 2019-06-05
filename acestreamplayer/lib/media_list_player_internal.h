#ifndef _LIBVLC_MEDIA_LIST_PLAYER_INTERNAL_H
#define _LIBVLC_MEDIA_LIST_PLAYER_INTERNAL_H 1

#include <vlc/libvlc.h>
#include <vlc/libvlc_media_list.h>
#include <vlc/libvlc_media_player.h>
#include <vlc/libvlc_media_list_player.h>

libvlc_media_list_t *libvlc_media_list_player_get_media_list( libvlc_media_list_player_t* );
libvlc_media_player_t *libvlc_media_list_player_get_media_player( libvlc_media_list_player_t* );
bool libvlc_media_player_acestream_media_start(  libvlc_media_list_player_t*, libvlc_media_t* );
void libvlc_media_list_player_set_fullstopped( libvlc_media_list_player_t* );

#endif