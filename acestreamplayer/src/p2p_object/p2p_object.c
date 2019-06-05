#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_modules.h>
#include <p2p_object.h>
#include <vlc_playlist.h>
#include "playlist/playlist_internal.h"
#include "p2p_object_internal.h"

#include <ctype.h>

static vlc_mutex_t global_lock = VLC_STATIC_MUTEX;

static p2p_uri_id_type_t get_uri_id( const char* );

#undef p2p_Get
p2p_object_t *p2p_Get (vlc_object_t *obj)
{
    p2p_object_t *p2p;
    libvlc_int_t *p_libvlc = obj->p_libvlc;

    vlc_mutex_lock (&global_lock);
    p2p = libvlc_priv(p_libvlc)->p_p2p;
    assert ( p2p != NULL );
    vlc_mutex_unlock (&global_lock);
    
    return p2p;
}

#undef p2p_Deactivate
void p2p_Deactivate(vlc_object_t *obj)
{
    msg_Dbg(obj, "deactivating p2p");
    libvlc_int_t *p_libvlc = obj->p_libvlc;
    if(p_libvlc != NULL) {
        p2p_object_t *p_p2p = libvlc_priv(p_libvlc)->p_p2p;
        if( p_p2p != NULL ) {
            p2p_Destroy( p_p2p );
            libvlc_priv(p_libvlc)->p_p2p = NULL;
        }
    }
}

p2p_uri_id_type_t p2p_GetIdTypeWithOptArray( const char *id, int optcount, const char* const* options )
{
    p2p_uri_id_type_t type = P2P_TYPE_UNSUPPORT;
    if( options ) {
        for( size_t i = 0; i < optcount; ++i ) {
            if( strsub( options[i], "ace-torrent" ) || strsub( options[i], "ts-torrent" ) ) {
                type = P2P_TYPE_TORRENT_URL;
                break;
            }
            else if( strsub( options[i], "ace-efile" ) || strsub( options[i], "ts-efile" ) ) {
                type = P2P_TYPE_EFILE;
                break;
            }
            else if( strsub( options[i], "ace-content-id" ) || strsub( options[i], "ts-content-id" ) ) {
                type = P2P_TYPE_PLAYER;
                break;
            }
            else if( strsub( options[i], "ace-info-hash" ) || strsub( options[i], "ts-info-hash" ) ) {
                type = P2P_TYPE_INFOHASH;
                break;
            }
        }
    }
    
    if( type == P2P_TYPE_UNSUPPORT ) {
        type = get_uri_id(id);
    }
    return type;
}

p2p_uri_id_type_t p2p_GetIdTypeWithOptString( const char *id, const char *options )
{
    p2p_uri_id_type_t type = P2P_TYPE_UNSUPPORT;
    if( options ) {
        if( strsub( options, "ace-torrent" ) || strsub( options, "ts-torrent" ) )
            type = P2P_TYPE_TORRENT_URL;
        else if( strsub( options, "ace-efile" ) || strsub( options, "ts-efile" ) )
            type = P2P_TYPE_EFILE;
        else if( strsub( options, "ace-content-id" ) || strsub( options, "ts-content-id" ) )
            type = P2P_TYPE_PLAYER;
        else if( strsub( options, "ace-info-hash" ) || strsub( options, "ts-info-hash" ) )
            type = P2P_TYPE_INFOHASH;
    }
    
    if( type == P2P_TYPE_UNSUPPORT ) {
        type = get_uri_id(id);
    }
    return type;
}

static int P2PDisplaySizeCallback(vlc_object_t *p_this, char const *psz_cmd, vlc_value_t oldval, vlc_value_t newval, void *data) {
    VLC_UNUSED(psz_cmd);  VLC_UNUSED(oldval);  VLC_UNUSED(newval);
    p2p_object_t *p_p2p = (p2p_object_t*)p_this;

    int w, h;
    var_GetCoords(p_p2p, "vout-display-size", &w, &h);
    msg_P2PLog( p_p2p, "[p2p_object.c] requesting new interactive because off resize %d %d", w, h );
    p2p_RequestLoadUrlAd(p_p2p, P2P_LOAD_URL_OVERLAY, 0);
    p2p_RequestLoadUrlAd(p_p2p, P2P_LOAD_URL_PAUSE, 0);
    if( w!=0 && h!=0 ) {
        p2p_RequestLoadUrlAd(p_p2p, P2P_LOAD_URL_STOP, 0);
    }
    
    return VLC_SUCCESS;
}

static void VariablesInit( p2p_object_t *p_p2p )
{
    var_Create( p_p2p, "auth", VLC_VAR_INTEGER );
    var_SetInteger( p_p2p, "auth", 1 );
    var_Create( p_p2p, "state", VLC_VAR_INTEGER );
    var_SetInteger( p_p2p, "state", (int)P2P_STATE_NOTLAUNCHED );
    var_Create( p_p2p, "info", VLC_VAR_STRING );
    var_SetString( p_p2p, "info", "" );
    var_Create( p_p2p, "status", VLC_VAR_STRING );
    var_SetString( p_p2p, "status", "" );
    var_Create( p_p2p, "status-raw", VLC_VAR_STRING );
    var_SetString( p_p2p, "status-raw", "" );
    var_Create( p_p2p, "error", VLC_VAR_STRING );
    var_SetString( p_p2p, "error", "" );
    var_Create( p_p2p, "engine-version", VLC_VAR_STRING );
    var_SetString( p_p2p, "engine-version", "" );
    var_Create( p_p2p, "engine-http-host", VLC_VAR_STRING );
    var_SetString( p_p2p, "engine-http-host", "127.0.0.1" );
    var_Create( p_p2p, "engine-http-port", VLC_VAR_INTEGER );
    var_SetInteger( p_p2p, "engine-http-port", 0 );
    
    var_Create( p_p2p, "vout-display-size", VLC_VAR_COORDS );
    var_SetCoords( p_p2p, "vout-display-size", 0, 0 );
    var_AddCallback( p_p2p, "vout-display-size", P2PDisplaySizeCallback, NULL );
    
    var_Create( p_p2p, "vout-display-fullscreen", VLC_VAR_BOOL );
    var_SetBool( p_p2p, "vout-display-fullscreen", false );
    
    var_Create( p_p2p, "livepos", VLC_VAR_P2P_LIVEPOS );    // for input
    var_Create( p_p2p, "showdialog", VLC_VAR_ADDRESS );        // show ad url
    
    var_Create( p_p2p, "clickurl", VLC_VAR_STRING );  // current clickurl
    var_SetString( p_p2p, "clickurl", "" );
    
    var_Create( p_p2p, "adparams", VLC_VAR_ADDRESS );        // ad params
    
    var_Create( p_p2p, "exit-fullscreen", VLC_VAR_VOID );  // trigger to exit fullscreen if needed
    var_Create( p_p2p, "show-userdata-dialog", VLC_VAR_VOID );  // trigger to show dialog
    var_Create( p_p2p, "show-playlist", VLC_VAR_VOID );  // trigger to show playlist
    
    var_Create( p_p2p, "item-will-replay", VLC_VAR_BOOL );
    var_SetBool( p_p2p, "item-will-replay", false );

    var_Create( p_p2p, "ad-skipped", VLC_VAR_BOOL );
    var_SetBool( p_p2p, "ad-skipped", false );
    
    var_Create( p_p2p, "load-url", VLC_VAR_ADDRESS ); // load url event

    var_Create( p_p2p, "showinfowindow", VLC_VAR_ADDRESS );
}

static void VariablesUninit( p2p_object_t *p_p2p )
{
    var_Destroy( p_p2p, "auth" );
    var_Destroy( p_p2p, "state" );
    var_Destroy( p_p2p, "info" );
    var_Destroy( p_p2p, "status" );
    var_Destroy( p_p2p, "status-raw" );
    var_Destroy( p_p2p, "error" );
    var_Destroy( p_p2p, "engine-version" );
    var_Destroy( p_p2p, "engine-http-host" );
    var_Destroy( p_p2p, "engine-http-port" );

    var_DelCallback( p_p2p, "vout-display-size", P2PDisplaySizeCallback, NULL );
    var_Destroy( p_p2p, "vout-display-size" );
    var_Destroy( p_p2p, "vout-display-fullscreen" );
    var_Destroy( p_p2p, "livepos" );
    var_Destroy( p_p2p, "showdialog" );
    var_Destroy( p_p2p, "clickurl" );
    var_Destroy( p_p2p, "adparams" );
    var_Destroy( p_p2p, "exit-fullscreen" );
    var_Destroy( p_p2p, "show-userdata-dialog");
    var_Destroy( p_p2p, "show-playlist");
    var_Destroy( p_p2p, "item-will-replay" );
    var_Destroy( p_p2p, "ad-skipped" );
    
    var_Destroy( p_p2p, "load-url" );

    var_Destroy( p_p2p, "showinfowindow" );
}

static void ClearMethods( p2p_object_t *p_p2p )
{
    p_p2p->pf_load_with_opt_array = NULL;
    p_p2p->pf_load_with_opt_string = NULL;
    p_p2p->pf_start = NULL;
    p_p2p->pf_stop = NULL;
    p_p2p->pf_duration = NULL;
    p_p2p->pf_playback = NULL;
    p_p2p->pf_get_pid = NULL;
    p_p2p->pf_get_cid = NULL;
    p_p2p->pf_save = NULL;
    p_p2p->pf_get_ad_url = NULL;
    p_p2p->pf_live_seek = NULL;
    p_p2p->pf_user_data = NULL;
    p_p2p->pf_user_data_mining = NULL;
    p_p2p->pf_info_window_response = NULL;
    p_p2p->pf_stat_event = NULL;
    p_p2p->pf_save_option = NULL;
    p_p2p->pf_set_callback = NULL;
    p_p2p->pf_register_load_url_ad_stat = NULL;
    p_p2p->pf_register_load_url_ad_event = NULL;
    p_p2p->pf_request_load_url_ad = NULL;
    p_p2p->pf_restart_last = NULL;
}

p2p_object_t *p2p_Create( vlc_object_t *p_parent )
{
    p2p_object_t *p_p2p;
    
    p_p2p = vlc_custom_create( p_parent, sizeof( *p_p2p ), "p2paccess" );
    if( !p_p2p ) {
        msg_P2PLog(p_parent, "[p2p_object.c] cannot allocate memory");
        return NULL;
    }
    
    VariablesInit( p_p2p );
    ClearMethods( p_p2p );
    
    vlc_mutex_init( &p_p2p->lock );
    p_p2p->p_module = module_need( p_p2p, "p2p", "p2p_access", true );
	if(!p_p2p->p_module)
	{
        msg_P2PLog(p_p2p, "[p2p_object.c] p2p_access creating error");
		vlc_object_release( p_p2p );
		return NULL;
	}

    p_p2p->p_vclick = p2p_InitVideoClick( p_p2p );
    libvlc_priv(p_parent->p_libvlc)->p_p2p = p_p2p;
    
    msg_P2PLog(p_p2p, "[p2p_object.c] p2p_object created");
    return p_p2p;
}

void p2p_Destroy( p2p_object_t *p_p2p )
{
    msg_P2PLog(p_p2p, "[p2p_object.c] destroying");
    p2p_UnInitVideoClick( p_p2p->p_vclick );
    p_p2p->p_vclick = NULL;
    module_unneed( p_p2p, p_p2p->p_module );

    ClearMethods( p_p2p );
    VariablesUninit( p_p2p );

    vlc_mutex_destroy( &p_p2p->lock );
    vlc_object_release( p_p2p );
}

void p2p_SetCallback( p2p_object_t *p_p2p, p2p_command_callback_type type, p2p_common_callback pf_callback, void *cb_data )
{
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_set_callback )
        p_p2p->pf_set_callback( p_p2p, type, pf_callback, cb_data );
    vlc_mutex_unlock( &p_p2p->lock );
}

int p2p_LoadWithOptArray( p2p_object_t *p_p2p, const char *id, 
        const char *name, p2p_uri_id_type_t type, bool async, 
        int optcount, const char* const *options, int add_mode, int callback_type, playlist_item_t *p_parent_node,
        bool playlist, bool start, 
        int developer, int affiliate, int zone, bool object_locked )
{
    int ret = VLC_EGENERIC;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_load_with_opt_array )
        ret = p_p2p->pf_load_with_opt_array( p_p2p, id, name, type, async, optcount, options, add_mode, callback_type, p_parent_node, playlist, start, developer, affiliate, zone, object_locked );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

int p2p_LoadWithOptString( p2p_object_t *p_p2p, const char *id, 
        const char *name, p2p_uri_id_type_t type, bool async, 
        const char *options,  int add_mode, int callback_type, playlist_item_t *p_parent_node,
        bool playlist, bool start,
        int developer, int affiliate, int zone, bool object_locked )
{
    int ret = VLC_EGENERIC;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_load_with_opt_string )
        ret = p_p2p->pf_load_with_opt_string( p_p2p, id, name, type, async, options, add_mode, callback_type, p_parent_node, playlist, start, developer, affiliate, zone, object_locked );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

bool p2p_Start( p2p_object_t *p_p2p, const char *id, const char *indexes, p2p_uri_id_type_t type, 
        int quality, int developer, int affiliate, int zone, int position_from )
{
    bool ret = false;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_start )
        ret = p_p2p->pf_start( p_p2p, id, indexes, type, quality, developer, affiliate, zone, position_from );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

bool p2p_Stop( p2p_object_t *p_p2p )
{
    bool ret = false;
    var_SetString( p_p2p, "info", "" );
    var_SetString( p_p2p, "status", "" );
    var_SetBool( p_p2p, "item-will-replay", false );
    var_SetBool( p_p2p, "ad-skipped", false );
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_stop )
        ret = p_p2p->pf_stop( p_p2p );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

bool p2p_Duration( p2p_object_t *p_p2p, const char *url, int duration )
{
    bool ret = false;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_duration )
        ret = p_p2p->pf_duration( p_p2p, url, duration );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

bool p2p_Playback( p2p_object_t *p_p2p, const char *url, p2p_playback_value_t playback )
{
    bool ret = false;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_playback )
        ret = p_p2p->pf_playback( p_p2p, url, playback );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

bool p2p_Skip( p2p_object_t *p_p2p, const char *url )
{
    bool ret = false;
    var_SetBool( p_p2p, "ad-skipped", true );
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_playback )
        ret = p_p2p->pf_playback( p_p2p, url, P2P_PLAYBACK_SKIP );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

char* p2p_GetPid( p2p_object_t *p_p2p, const char *infohash, int developer, int affiliate, int zone )
{
    char *ret = NULL;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_get_pid )
        ret = p_p2p->pf_get_pid( p_p2p, infohash, developer, affiliate, zone );
    vlc_mutex_unlock( &p_p2p->lock );
    return ret;
}

char* p2p_GetCid( p2p_object_t *p_p2p, const char *infohash, const char *checksum, int developer, int affiliate, int zone )
{
    char *ret = NULL;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_get_cid ) 
        ret = p_p2p->pf_get_cid( p_p2p, infohash, checksum, developer, affiliate, zone );
    vlc_mutex_unlock( &p_p2p->lock );
    return ret;
}

bool p2p_Save(p2p_object_t *p_p2p, const char *infohash, int index, const char*path )
{
    bool ret = false;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_save )
        ret = p_p2p->pf_save( p_p2p, infohash, index, path );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

bool p2p_GetAdUrl(p2p_object_t *p_p2p, const char *infohash, const char *action, int width, int height)
{
    bool ret = false;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_get_ad_url ) 
        ret = p_p2p->pf_get_ad_url( p_p2p, infohash, action, width, height );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

bool p2p_LiveSeek(p2p_object_t *p_p2p, int pos)
{
    bool ret = false;
    var_SetBool( p_p2p, "item-will-replay", true );
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_live_seek )
        ret = p_p2p->pf_live_seek( p_p2p, pos );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

bool p2p_UserData( p2p_object_t *p_p2p, int gender, int age )
{
    bool ret = false;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_user_data )
        ret = p_p2p->pf_user_data( p_p2p, gender, age );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

bool p2p_UserDataMining( p2p_object_t *p_p2p, int value )
{
    bool ret = false;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_user_data_mining )
        ret = p_p2p->pf_user_data_mining( p_p2p, value );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

bool p2p_InfoWindowsResponse( p2p_object_t *p_p2p, const char *type, int button)
{
    bool ret = false;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_info_window_response )
        ret = p_p2p->pf_info_window_response( p_p2p, type, button );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

bool p2p_StatEvent(p2p_object_t *p_p2p, p2p_statistics_event_type_t event, int value)
{
    bool ret = false;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_stat_event )
        ret = p_p2p->pf_stat_event( p_p2p, event, value );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

bool p2p_SaveOption(p2p_object_t *p_p2p, const char *infohash, const char *name, const char *value)
{
    bool ret = false;
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_save_option )
        ret = p_p2p->pf_save_option( p_p2p, infohash, name, value );
    vlc_mutex_unlock( &p_p2p->lock );
	return ret;
}

void p2p_VideoClickActivate( p2p_object_t *p_p2p, bool b_activate )
{
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->p_vclick ) {
        if( b_activate ) {
            char *clickurl = var_GetString( p_p2p, "clickurl" );
            if( clickurl ) {
                p2p_SetVideoClickState( p_p2p->p_vclick, b_activate, clickurl );
                free( clickurl );
            }
        }
        else
            p2p_SetVideoClickState( p_p2p->p_vclick, b_activate, NULL );
    }
    vlc_mutex_unlock( &p_p2p->lock );
}

void p2p_RegisterLoadUrlAdStatistics(p2p_object_t *p_p2p, p2p_load_url_type_t type, p2p_load_url_statistics_event_type_t event_type, const char *id)
{
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_register_load_url_ad_stat )
        p_p2p->pf_register_load_url_ad_stat( p_p2p, (int)type, (int)event_type, id );
    vlc_mutex_unlock( &p_p2p->lock );
}

void p2p_RequestLoadUrlAd(p2p_object_t *p_p2p, p2p_load_url_type_t type, int group_id)
{
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_request_load_url_ad )
        p_p2p->pf_request_load_url_ad( p_p2p, (int)type, group_id );
    vlc_mutex_unlock( &p_p2p->lock );
}

void p2p_RegisterLoadUrlAdEvent(p2p_object_t *p_p2p, p2p_load_url_type_t type, const char *event_type, const char *id)
{
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_register_load_url_ad_event )
        p_p2p->pf_register_load_url_ad_event( p_p2p, (int)type, event_type, id );
    vlc_mutex_unlock( &p_p2p->lock );
}

void p2p_RestartLast(p2p_object_t *p_p2p)
{
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->pf_restart_last )
        p_p2p->pf_restart_last( p_p2p );
    vlc_mutex_unlock( &p_p2p->lock );
}

static p2p_uri_id_type_t get_uri_id( const char *id )
{
    if(!id) {
        return P2P_TYPE_UNSUPPORT;
    }

    char *acestream_proto = strstr( id, "acestream://" );
    if( acestream_proto ) {
        if(strlen(id) ==  52)
            return P2P_TYPE_PLAYER;
        else if(strlen(id) == 53 && id[52]=='/')
            return P2P_TYPE_PLAYER;
        return P2P_TYPE_UNSUPPORT;
    }

    char *last_point = strrchr( id, '.' );
	if( !last_point ) {
        if( strlen( id ) == 40 && !strchr( id, ':' ) ) {
            return P2P_TYPE_PLAYER;
        }
        return P2P_TYPE_UNSUPPORT;
	}

    char *id_extension = (char*)malloc( strlen(id) + 1 );
	int last = last_point - id + 1;
	strncpy_from( id_extension, id, last, ( strlen(id) + 1 ) - last );
    for( int i = 0; i < strlen( id_extension ); i++ ) {
		id_extension[i] = tolower( id_extension[i] );
	}
	id_extension[strlen(id_extension)]='\0';

    p2p_uri_id_type_t ret = P2P_TYPE_UNSUPPORT;
	if( !strcmp(id_extension, "acemedia") ) {
		ret = P2P_TYPE_EFILE;
	}
	else if( !strcmp(id_extension,"torrent") || !strcmp(id_extension,"acelive") || !strcmp(id_extension,"acestream") || !strcmp(id_extension,"tslive") ) {
        ret = P2P_TYPE_TORRENT_URL;
	}
	free( id_extension );
	return ret;
}
