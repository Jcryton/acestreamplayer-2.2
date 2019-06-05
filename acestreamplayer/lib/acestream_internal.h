#ifndef _LIBVLC_ACESTREAM_INTERNAL_H
#define _LIBVLC_ACESTREAM_INTERNAL_H 1

#include <vlc/vlc.h>
#include <vlc/libvlc_acestream.h>

#include <vlc_common.h>
#include <p2p_object.h>

struct libvlc_acestream_object_t
{
    libvlc_instance_t *p_libvlc_instance;
	libvlc_event_manager_t *p_event_manager;
	libvlc_media_list_player_t *p_mlist_player;
    
	int i_refcount;  	
	vlc_mutex_t object_lock;
    
    bool b_preplay_got;
};

#endif
