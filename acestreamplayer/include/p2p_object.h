/*
 p2p_object.h
 
 Copyright (C) 2012-2013 Iurii Svyryd
 Author: Iurii Svyryd <usvirid at gmail dot com>
 
 This file is part of TS player and ACE player HD (VLC based).
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __P2P_OBJECT_HEADER__
#define __P2P_OBJECT_HEADER__

#include "vlc_common.h"

# ifdef __cplusplus
extern "C" {
# endif

#ifdef TORRENT_STREAM
#define P2P_PRODUCT_NAME "Torrent Stream"
#define P2P_CONFIG_DIR ".Torrent Stream"
#define P2P_APP_NAME "TS Player"
#else
#define P2P_PRODUCT_NAME "Ace Stream"
#define P2P_CONFIG_DIR ".ACEStream"
#define P2P_APP_NAME "Ace Player HD"
#endif

#define P2P_APP_VERSION "2.2.5.1"
#define P2P_STD_VERSION "2.2.5.1"

typedef enum p2p_state_t {
    P2P_STATE_NOTLAUNCHED = -1,
    P2P_STATE_IDLE = 0,
    P2P_STATE_PREBUFFERING = 1,
    P2P_STATE_DOWNLOADING = 2,
    P2P_STATE_BUFFERING = 3,
    P2P_STATE_COMPLETED = 4, 
    P2P_STATE_HASH_CHECKING = 5,
    P2P_STATE_ERROR = 6,
    P2P_STATE_CONNECTING = 7,
    P2P_STATE_LOADING = 8,
    P2P_STATE_LAUNCHING = 9
} p2p_state_t;

typedef enum p2p_uri_id_type_t {
    P2P_TYPE_UNSUPPORT = -1,
    P2P_TYPE_TORRENT_URL = 0,
    P2P_TYPE_DIRECT_URL = 1,
    P2P_TYPE_INFOHASH = 2,
    P2P_TYPE_PLAYER = 3,
    P2P_TYPE_RAW = 4,
    P2P_TYPE_EFILE = 5
} p2p_uri_id_type_t;

typedef enum p2p_save_format_t {
    P2P_SAVE_UNSAVEABLE = 0,
    P2P_SAVE_PLAIN = 1,
    P2P_SAVE_ENCRYPTED = 2
} p2p_save_format_t;

typedef enum p2p_play_content_type_t {
    P2P_PLAY_UNDF = -1,
    P2P_PLAY_MAIN,
    P2P_PLAY_AD,
    P2P_PLAY_INTERRUPTABLE_AD,
    P2P_PLAY_STREAM
} p2p_play_content_type_t;

typedef enum p2p_statistics_event_type_t {
    P2P_STAT_PLAY,
    P2P_STAT_PAUSE,
    P2P_STAT_SEEK,
    P2P_STAT_STOP
} p2p_statistics_event_type_t;

enum p2p_load_type {
    p2p_LoadAsync = true,
    p2p_LoadSync = false
};

enum p2p_load_desktop_add_type {
    p2p_LoadAddInput = 0,
    p2p_LoadAddToNode = 1
};

typedef enum p2p_playback_value_t {
    P2P_PLAYBACK_UNDF = -1,
    P2P_PLAYBACK_0 = 0,
    P2P_PLAYBACK_25 = 25,
    P2P_PLAYBACK_50 = 50,
    P2P_PLAYBACK_75 = 75,
    P2P_PLAYBACK_100 = 100,
    P2P_PLAYBACK_SKIP = 101
} p2p_playback_value_t;

typedef enum p2p_load_url_type_t {
    P2P_LOAD_URL_UNDF = -1,
    P2P_LOAD_URL_PAUSE,
    P2P_LOAD_URL_STOP,
    P2P_LOAD_URL_OVERLAY,
    P2P_LOAD_URL_PREROLL,
    P2P_LOAD_URL_SLIDER,
    P2P_LOAD_URL_HIDDEN,
    P2P_LOAD_URL_PREPLAY = 6,
    P2P_LOAD_URL_WEBSTAT_PLAY,
    P2P_LOAD_URL_WEBSTAT_PAUSE,
    P2P_LOAD_URL_WEBSTAT_STOP,
    P2P_LOAD_URL_WEBSTAT_FULLSCREEN
} p2p_load_url_type_t;

typedef enum p2p_load_url_statistics_event_type_t {
    P2P_LOAD_URL_STAT_EVENT_IMPRESSION,
    P2P_LOAD_URL_STAT_EVENT_CLOSE,
    P2P_LOAD_URL_STAT_EVENT_COMPLETE,
    P2P_LOAD_URL_STAT_EVENT_ERROR,
    P2P_LOAD_URL_STAT_EVENT_COMPLETE_HIDDEN,
    P2P_LOAD_URL_STAT_EVENT_ERROR_HIDDEN,
} p2p_load_url_statistics_event_type_t;

typedef enum p2p_infowindow_button_action_t {
    P2P_IW_BTN_ACTION_CLOSE = 1,
    P2P_IW_BTN_ACTION_OPENLINK = 2,
    P2P_IW_BTN_ACTION_PLAY = 4,
    P2P_IW_BTN_ACTION_STOP = 8,
    P2P_IW_BTN_ACTION_SENDEVENT = 16,
    P2P_IW_BTN_ACTION_MINIGACTIVATE = 32,
} p2p_infowindow_button_action_t;

// callback structs
struct p2p_load_item_t {
    input_item_t *p_input_item;
    bool add_to_playlist;
    int add_mode;
    bool need_start;
    int desktop_add_type;
    playlist_item_t *p_parent_node;
    bool object_locked;
};

struct p2p_play_item_t {
    const char *url;
    int start_position;
    p2p_play_content_type_t play_content_type;
    const char *deinterlace;
};

struct p2p_cansave_item_t {
    const char *infohash;
    int index;
    p2p_save_format_t format;
};

struct p2p_load_url_item_t {
    p2p_load_url_type_t type;
    const char *id;
    const char *url;
    int width;
    int height;
    int left;
    int top;
    int right;
    int bottom;
    bool allow_dialogs;
    bool allow_window_open;
    bool enable_flash;
    int cookies;
    const char *embed_scripts;
    const char *embed_code;
    
    // pause+stop
    bool preload;
    int fullscreen;
    // overlay
    const char *content_type;
    const char *creative_type;
    const char *click_url;    
    
    // hidden
    int close_after_seconds;
    int show_time;
    
    bool start_hidden;
    int user_agent;
    bool clear;
    bool url_filter;
    
    int group_id;
    bool useIE;
};










struct p2p_showdialog_item_t {
    const char *title;
    const char *text;
};

struct p2p_showinfowindow_item_t {
    const char *type;
    const char *text;
    int height;
    int buttons;
    int btn1_action;
    const char *btn1_text;
    const char *btn1_url;
    int btn2_action;
    const char *btn2_text;
    const char *btn2_url;
};

struct p2p_ad_params_t {
    const char *url;
    const char *text;
    const char *skipoffset;
    const char *noadsurl;
    const char *noadstext;
    const char *adinfotext;
};

#define P2P_DEV 6
#define P2P_AFF 0
#define P2P_ZONE 0

typedef enum p2p_command_callback_type {
    P2P_LOAD_CALLBACK = 0,
    P2P_PLAY_CALLBACK,
    P2P_PAUSE_CALLBACK,
    P2P_RESUME_CALLBACK,
    P2P_STOP_CALLBACK,
    P2P_CANSAVE_CALLBACK,
    // ...
    P2P_MAX_CALLBACK
} p2p_command_callback_type;

typedef int (*p2p_common_callback) (p2p_object_t*, void*, void*);
typedef struct p2p_video_click_t p2p_video_click_t;

struct p2p_object_t {
    VLC_COMMON_MEMBERS
    
    module_t *p_module;
    p2p_object_sys_t *p_sys;
    p2p_video_click_t *p_vclick;
    vlc_mutex_t lock;
    
    void (*pf_set_callback) (p2p_object_t*, p2p_command_callback_type, p2p_common_callback, void*);
    
    int (*pf_load_with_opt_array) (p2p_object_t*, const char* /*id*/, const char*/*name*/, p2p_uri_id_type_t/*type*/, bool /*async=true*/, int/*optcount*/, const char* const*/*optionsarr*/, int /*add mode*/, int /*callback type*/, playlist_item_t* /*parent node*/, bool/*playlist/ml*/, bool/*startplaying*/, int/*developer*/, int/*affiliate*/, int/*zone*/, bool /*playlist locked*/);
    int (*pf_load_with_opt_string) (p2p_object_t*, const char*/*id*/, const char*/*name*/, p2p_uri_id_type_t/*type*/, bool /*async=true*/, const char*/*optstring*/, int /*add mode*/, int /*callback type*/, playlist_item_t* /*parent node*/, bool/*playlist/ml*/, bool/*startplaying*/, int/*developer*/, int/*affiliate*/, int/*zone*/, bool /*playlist locked*/);
    
    bool (*pf_start) (p2p_object_t*, const char* /*id*/, const char* /*indexes*/, p2p_uri_id_type_t/*type*/, int /*quality*/, int/*developer*/, int/*affiliate*/, int/*zone*/, int/*start from position*/);
    bool (*pf_stop) (p2p_object_t*);
    
    bool (*pf_duration) (p2p_object_t*, const char* /*url*/, int/*duration*/);
	bool (*pf_playback) (p2p_object_t*, const char* /*url*/, p2p_playback_value_t/*playback*/);
    
    char* (*pf_get_pid) (p2p_object_t*, const char*/*infohash*/, int/*developer*/, int/*affiliate*/, int/*zone*/);
    char* (*pf_get_cid) (p2p_object_t*, const char*/*infohash*/, const char*/*checksum*/, int/*developer*/, int/*affiliate*/, int/*zone*/);
    
    bool (*pf_save) (p2p_object_t*, const char*/*infohash*/, int/*index*/, const char*/*save to path*/);
    bool (*pf_get_ad_url) (p2p_object_t*, const char*/*infohash*/, const char*/*action*/, int /*video window width*/, int /*video window height*/);
    bool (*pf_live_seek) (p2p_object_t*, int/*position*/);
    bool (*pf_user_data) (p2p_object_t*, int/*gender*/, int/*age*/);
    bool (*pf_user_data_mining) (p2p_object_t*, int/*value*/);
    bool (*pf_info_window_response) (p2p_object_t*, const char*/*type*/, int/*button*/);
    
    bool (*pf_stat_event) (p2p_object_t*, p2p_statistics_event_type_t/*event type*/, int/*value*/);
    bool (*pf_save_option) (p2p_object_t*, const char*, const char*, const char*);
    
    void (*pf_register_load_url_ad_stat) (p2p_object_t*, p2p_load_url_type_t, p2p_load_url_statistics_event_type_t, const char*);
    void (*pf_request_load_url_ad) (p2p_object_t*, p2p_load_url_type_t, int);
    void (*pf_register_load_url_ad_event) (p2p_object_t*, p2p_load_url_type_t, const char*, const char*);
    
    void (*pf_restart_last) (p2p_object_t*);
};

VLC_API p2p_object_t *p2p_Get( vlc_object_t * );
#define p2p_Get( a ) p2p_Get( VLC_OBJECT(a) )
VLC_API void p2p_Deactivate( vlc_object_t * );
#define p2p_Deactivate( a ) p2p_Deactivate( VLC_OBJECT(a) )

VLC_API void p2p_SetCallback(p2p_object_t*, p2p_command_callback_type, p2p_common_callback, void*);

VLC_API p2p_uri_id_type_t p2p_GetIdTypeWithOptArray( const char*, int, const char* const* );
VLC_API p2p_uri_id_type_t p2p_GetIdTypeWithOptString( const char*, const char* );

VLC_API int p2p_LoadWithOptArray( p2p_object_t*, const char*, const char*, p2p_uri_id_type_t, bool, int, const char* const*, int, int, playlist_item_t*, bool, bool, int, int, int, bool );
VLC_API int p2p_LoadWithOptString( p2p_object_t*, const char*, const char*, p2p_uri_id_type_t, bool, const char*, int, int, playlist_item_t*, bool, bool, int, int, int, bool );

VLC_API bool p2p_Start( p2p_object_t*, const char*, const char*, p2p_uri_id_type_t, int, int, int, int, int );
VLC_API bool p2p_Stop( p2p_object_t* );

VLC_API bool p2p_Duration( p2p_object_t*, const char*, int );
VLC_API bool p2p_Playback( p2p_object_t*, const char*, p2p_playback_value_t );
VLC_API bool p2p_Skip( p2p_object_t*, const char* );

VLC_API char* p2p_GetPid( p2p_object_t*, const char*, int, int, int );
VLC_API char* p2p_GetCid( p2p_object_t*, const char*, const char*, int, int, int );

VLC_API bool p2p_Save(p2p_object_t*, const char*, int, const char*);
VLC_API bool p2p_GetAdUrl(p2p_object_t*, const char*, const char*, int, int);
VLC_API bool p2p_LiveSeek(p2p_object_t*, int);
VLC_API bool p2p_UserData( p2p_object_t*, int, int );
VLC_API bool p2p_UserDataMining( p2p_object_t*, int );
VLC_API bool p2p_InfoWindowsResponse( p2p_object_t*, const char*, int );

VLC_API bool p2p_StatEvent(p2p_object_t*, p2p_statistics_event_type_t, int);
VLC_API bool p2p_SaveOption(p2p_object_t*, const char*, const char*, const char*);

VLC_API void p2p_VideoClickActivate( p2p_object_t*, bool );

VLC_API void p2p_RegisterLoadUrlAdStatistics(p2p_object_t*, p2p_load_url_type_t, p2p_load_url_statistics_event_type_t, const char*);
VLC_API void p2p_RequestLoadUrlAd(p2p_object_t*, p2p_load_url_type_t, int);
VLC_API void p2p_RegisterLoadUrlAdEvent(p2p_object_t*, p2p_load_url_type_t, const char*, const char*);

VLC_API void p2p_RestartLast(p2p_object_t*);

# ifdef __cplusplus
}
# endif

#endif