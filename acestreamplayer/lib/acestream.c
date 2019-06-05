#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>

#include <vlc/libvlc.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_media_list.h>
#include <vlc/libvlc_media_player.h>
#include <vlc/libvlc_media_list_player.h>
#include <vlc/libvlc_acestream.h>
#include <vlc/libvlc_events.h>

#include <vlc_input.h>
#include <vlc_input_item.h>

#include "libvlc_internal.h"
#include "acestream_internal.h"
#include "media_internal.h"
#include "media_list_internal.h"
#include "media_list_player_internal.h"
#include "media_player_internal.h"

static int acestream_auth_changed( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( oldval ); VLC_UNUSED( p_this ); VLC_UNUSED( psz_cmd );
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
    libvlc_event_t event;

    event.type = libvlc_AcestreamAuth;
    event.u.acestream_auth.auth = newval.i_int;

    libvlc_event_send( p_ace->p_event_manager, &event );
    return VLC_SUCCESS;
}

static int acestream_state_changed( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( oldval ); VLC_UNUSED( p_this ); VLC_UNUSED( psz_cmd );
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
    libvlc_event_t event;

    event.type = libvlc_AcestreamState;
    event.u.acestream_state.state = newval.i_int;

    libvlc_event_send( p_ace->p_event_manager, &event );
    return VLC_SUCCESS;
}

static int acestream_status_changed( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( oldval ); VLC_UNUSED( p_this ); VLC_UNUSED( psz_cmd );
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
    libvlc_event_t event;

    event.type = libvlc_AcestreamStatus;
    event.u.acestream_status.status = newval.psz_string;

    libvlc_event_send( p_ace->p_event_manager, &event );
    return VLC_SUCCESS;
}

static int acestream_status_raw_changed( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( oldval ); VLC_UNUSED( p_this ); VLC_UNUSED( psz_cmd );
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
    libvlc_event_t event;

    event.type = libvlc_AcestreamStatusRaw;
    event.u.acestream_status_raw.status = newval.psz_string;

    libvlc_event_send( p_ace->p_event_manager, &event );
    return VLC_SUCCESS;
}

static int acestream_info_changed( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( oldval ); VLC_UNUSED( p_this ); VLC_UNUSED( psz_cmd );
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
    libvlc_event_t event;

    event.type = libvlc_AcestreamInfo;
    event.u.acestream_info.info = newval.psz_string;

    libvlc_event_send( p_ace->p_event_manager, &event );
    return VLC_SUCCESS;
}

static int acestream_error_changed( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( oldval ); VLC_UNUSED( p_this ); VLC_UNUSED( psz_cmd );
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
    libvlc_event_t event;

    event.type = libvlc_AcestreamError;
    event.u.acestream_error.error = newval.psz_string;

    libvlc_event_send( p_ace->p_event_manager, &event );
    return VLC_SUCCESS;
}

static int acestream_showuserdata_dialog( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( p_this ); VLC_UNUSED( oldval ); VLC_UNUSED( newval ); VLC_UNUSED( psz_cmd );
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
    libvlc_event_t event;
    event.type = libvlc_AcestreamShowUserDataDialog;
    libvlc_event_send( p_ace->p_event_manager, &event );
    return VLC_SUCCESS;
}

static int acestream_showplaylist( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( p_this ); VLC_UNUSED( oldval ); VLC_UNUSED( newval ); VLC_UNUSED( psz_cmd );
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
    libvlc_event_t event;
    event.type = libvlc_AcestreamShowPlaylist;
    libvlc_event_send( p_ace->p_event_manager, &event );
    return VLC_SUCCESS;
}

static int acestream_showerror_dialog( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( oldval ); VLC_UNUSED( p_this ); VLC_UNUSED( psz_cmd );
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
    libvlc_event_t event;
    event.type = libvlc_AcestreamShowErrorDialog;
    
    p2p_showdialog_item_t *p_showdialog = (p2p_showdialog_item_t *)newval.p_address;
    
    event.u.acestream_showerrordialog.title = p_showdialog->title;
    event.u.acestream_showerrordialog.msg = p_showdialog->text;

    libvlc_event_send( p_ace->p_event_manager, &event );
    return VLC_SUCCESS;
}

static int acestream_showinfowindow( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( oldval ); VLC_UNUSED( p_this ); VLC_UNUSED( psz_cmd );
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
    libvlc_event_t event;
    event.type = libvlc_AcestreamShowInfoWindow;
    
    if(newval.p_address) {
        p2p_showinfowindow_item_t *p_showinfownd = (p2p_showinfowindow_item_t *)newval.p_address;
        event.u.acestream_showinfowindow.type = p_showinfownd->type;
        event.u.acestream_showinfowindow.text = p_showinfownd->text;
        event.u.acestream_showinfowindow.height = p_showinfownd->height;
        event.u.acestream_showinfowindow.buttons = p_showinfownd->buttons;
        event.u.acestream_showinfowindow.btn1_action = p_showinfownd->btn1_action;
        event.u.acestream_showinfowindow.btn1_text = p_showinfownd->btn1_text;
        event.u.acestream_showinfowindow.btn1_url = p_showinfownd->btn1_url;
        event.u.acestream_showinfowindow.btn2_action = p_showinfownd->btn2_action;
        event.u.acestream_showinfowindow.btn2_text = p_showinfownd->btn2_text;
        event.u.acestream_showinfowindow.btn2_url = p_showinfownd->btn2_url;

        libvlc_event_send( p_ace->p_event_manager, &event );
    }
    return VLC_SUCCESS;
}

static int acestream_loadurl( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( oldval ); VLC_UNUSED( p_this ); VLC_UNUSED( psz_cmd );
    
    if(newval.p_address) {
        libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
        libvlc_event_t event;
    
        p2p_load_url_item_t *p_loadurl = (p2p_load_url_item_t *)newval.p_address;
        
        if(p_loadurl->clear) {
            event.type = libvlc_AcestreamClearLoadUrl;
            event.u.acestream_clearloadurl.type = p_loadurl->type;
        }
        else {
            event.type = libvlc_AcestreamLoadUrl;
            event.u.acestream_loadurl.type = p_loadurl->type;
            event.u.acestream_loadurl.id = p_loadurl->id;
            event.u.acestream_loadurl.url = p_loadurl->url;
            event.u.acestream_loadurl.width = p_loadurl->width;
            event.u.acestream_loadurl.height = p_loadurl->height;
            event.u.acestream_loadurl.left = p_loadurl->left;
            event.u.acestream_loadurl.top = p_loadurl->top;
            event.u.acestream_loadurl.right = p_loadurl->right;
            event.u.acestream_loadurl.bottom = p_loadurl->bottom;
            event.u.acestream_loadurl.allow_dialogs = p_loadurl->allow_dialogs;
            event.u.acestream_loadurl.allow_window_open = p_loadurl->allow_window_open;
            event.u.acestream_loadurl.enable_flash = p_loadurl->enable_flash;
            event.u.acestream_loadurl.cookies = p_loadurl->cookies;
            event.u.acestream_loadurl.embed_scripts = p_loadurl->embed_scripts;
            event.u.acestream_loadurl.embed_code = p_loadurl->embed_code;
            
            event.u.acestream_loadurl.preload = p_loadurl->preload;
            event.u.acestream_loadurl.fullscreen = p_loadurl->fullscreen;
            
            event.u.acestream_loadurl.content_type = p_loadurl->content_type;
            event.u.acestream_loadurl.creative_type = p_loadurl->creative_type;
            event.u.acestream_loadurl.click_url = p_loadurl->click_url;
            
            event.u.acestream_loadurl.user_agent = p_loadurl->user_agent;
            event.u.acestream_loadurl.close_after_seconds = p_loadurl->close_after_seconds;
            event.u.acestream_loadurl.show_time = p_loadurl->show_time;
            
            event.u.acestream_loadurl.start_hidden = p_loadurl->start_hidden;
            event.u.acestream_loadurl.url_filter = p_loadurl->url_filter;
            event.u.acestream_loadurl.group_id = p_loadurl->group_id;
            
            event.u.acestream_loadurl.useIE = p_loadurl->useIE;
            
            msg_P2PLog(p_this, "[acestream_loadurl] load url callback with type %d", p_loadurl->type);
            if(p_loadurl->type == libvlc_ace_loadurl_Preplay) {
                p_ace->b_preplay_got = true;
            }
        }
        libvlc_event_send( p_ace->p_event_manager, &event );
    }
    
    return VLC_SUCCESS;
}

static int acestream_adparams( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( oldval ); VLC_UNUSED( p_this ); VLC_UNUSED( psz_cmd );
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
    libvlc_event_t event;

    p2p_ad_params_t *p_ad_params = (p2p_ad_params_t *)newval.p_address;
    
    event.type = libvlc_AcestreamAdParams;
    event.u.acestream_adparams.clickurl = p_ad_params->url;
    event.u.acestream_adparams.clicktext = p_ad_params->text;
    event.u.acestream_adparams.skipoffset = p_ad_params->skipoffset;
    event.u.acestream_adparams.noadsurl = p_ad_params->noadsurl;
    event.u.acestream_adparams.noadstext = p_ad_params->noadstext;
    event.u.acestream_adparams.adinfotext = p_ad_params->adinfotext;
    libvlc_event_send( p_ace->p_event_manager, &event );
    return VLC_SUCCESS;
}

static int acestream_exit_fullscreen( vlc_object_t * p_this, char const * psz_cmd, vlc_value_t oldval, vlc_value_t newval, void * p_userdata )
{
    VLC_UNUSED( oldval ); VLC_UNUSED( p_this ); VLC_UNUSED( psz_cmd );
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t* )p_userdata;
    
    libvlc_event_t event;
    event.type = libvlc_AcestreamForceExitFullscreen;
    libvlc_event_send( p_ace->p_event_manager, &event );
    
    return VLC_SUCCESS;
}

static int acestream_load_callback( p2p_object_t *p_p2p, void *p_callback_item, void *p_data )
{
    libvlc_acestream_object_t *p_ace = (libvlc_acestream_object_t*)p_data;
    p2p_load_item_t *p_load_item = (p2p_load_item_t*)p_callback_item;
    int ret = -1;
    
    msg_P2PLog( p_p2p, "[acestream_load_callback] load callback..." );
    if( p_load_item ) {
        if( p_load_item->p_input_item ) {
            input_item_t *p_input_item_copy = input_item_CopyFull( p_load_item->p_input_item );
            
            if( !p_load_item->object_locked )
                vlc_mutex_lock( &p_ace->object_lock );

            libvlc_media_t *p_media = libvlc_media_new_from_input_item( p_ace->p_libvlc_instance, p_input_item_copy );
            if( p_media ) {
                libvlc_media_list_t *p_mlist = libvlc_media_list_player_get_media_list( p_ace->p_mlist_player );
                
                libvlc_media_list_lock( p_mlist );
                if( !libvlc_media_list_add_media( p_mlist, p_media ) )
                    ret = libvlc_media_list_count( p_mlist ) - 1;
                libvlc_media_list_unlock( p_mlist );
                libvlc_media_release( p_media );
            }
            
            if( !p_load_item->object_locked )
                vlc_mutex_unlock( &p_ace->object_lock );

            vlc_gc_decref( p_input_item_copy );
        }
    }
    return ret;
}

static int acestream_play_callback( p2p_object_t *p_p2p, void *p_callback_item, void *p_data )
{
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t*)p_data;
    p2p_play_item_t *p_play_item = (p2p_play_item_t*)p_callback_item;

    msg_P2PLog( p_p2p, "[acestream_play_callback] play callback..." );
    if( p_play_item ) {
        vlc_mutex_lock( &p_ace->object_lock );
        libvlc_media_player_t *p_mi = libvlc_media_list_player_get_media_player( p_ace->p_mlist_player );
        vlc_mutex_unlock( &p_ace->object_lock );
        libvlc_media_t *p_md = libvlc_media_player_get_media( p_mi );
        
        if( p_md ) {
            if( libvlc_media_is_acestream_media(p_md) ) {
                input_item_SetP2PPlayCommandFlags( p_md->p_input_item, 
                        p_play_item->play_content_type == P2P_PLAY_AD,
                        p_play_item->play_content_type == P2P_PLAY_INTERRUPTABLE_AD,
                        p_play_item->play_content_type == P2P_PLAY_STREAM );

                var_SetFloat( p_mi, "start-position", p_play_item->start_position );
                var_SetString( p_mi, "start-deinterlace", p_play_item->deinterlace );
                input_item_SetURI( p_md->p_input_item, p_play_item->url );
            }
            libvlc_media_release(p_md);
            libvlc_media_player_play(p_mi);
        }
    }
    
    return VLC_SUCCESS;
}

static int acestream_stop_callback( p2p_object_t *p_p2p, void *p_callback_item, void *p_data )
{
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t*)p_data;

    msg_P2PLog( p_p2p, "[acestream_stop_callback] stop callback..." );    
    vlc_mutex_lock( &p_ace->object_lock );
    libvlc_media_player_t *p_mi = libvlc_media_list_player_get_media_player( p_ace->p_mlist_player );
    vlc_mutex_unlock( &p_ace->object_lock );
    libvlc_media_player_stop( p_mi );

    return VLC_SUCCESS;
}

static int acestream_cansave_callback( p2p_object_t *p_p2p, void *p_callback_item, void *p_data )
{
    libvlc_acestream_object_t *p_ace = ( libvlc_acestream_object_t*)p_data;
    p2p_cansave_item_t *p_cansave_item = (p2p_cansave_item_t*)p_callback_item;

    msg_P2PLog( p_p2p, "[acestream_cansave_callback] cansave callback..." );    
    if( p_cansave_item ) {
        vlc_mutex_lock( &p_ace->object_lock );
        libvlc_media_list_t *p_mlist = libvlc_media_list_player_get_media_list( p_ace->p_mlist_player );
        vlc_mutex_unlock( &p_ace->object_lock );
        
        libvlc_media_list_lock( p_mlist );
        libvlc_media_list_update_saveable( p_mlist, p_cansave_item->infohash, p_cansave_item->index, p_cansave_item->format );
        libvlc_media_list_unlock( p_mlist );
    }
    
    return VLC_SUCCESS;
}
static void bind_callbacks( libvlc_acestream_object_t *p_ace )
{
    p2p_object_t *p_p2p = getP2P( p_ace->p_libvlc_instance );
    if( p_p2p ) {
        var_AddCallback( p_p2p, "auth", acestream_auth_changed, p_ace );
        var_AddCallback( p_p2p, "state", acestream_state_changed, p_ace );
        var_AddCallback( p_p2p, "info", acestream_info_changed, p_ace );
        var_AddCallback( p_p2p, "status", acestream_status_changed, p_ace );
        var_AddCallback( p_p2p, "status-raw", acestream_status_raw_changed, p_ace );
        var_AddCallback( p_p2p, "error", acestream_error_changed, p_ace );
        var_AddCallback( p_p2p, "show-userdata-dialog", acestream_showuserdata_dialog, p_ace );
        var_AddCallback( p_p2p, "show-playlist", acestream_showplaylist, p_ace );
        var_AddCallback( p_p2p, "showdialog", acestream_showerror_dialog, p_ace );
        var_AddCallback( p_p2p, "adparams", acestream_adparams, p_ace );
        var_AddCallback( p_p2p, "exit-fullscreen", acestream_exit_fullscreen, p_ace );
        var_AddCallback( p_p2p, "load-url", acestream_loadurl, p_ace );
        var_AddCallback( p_p2p, "showinfowindow", acestream_showinfowindow, p_ace );
        
        p2p_SetCallback( p_p2p, P2P_LOAD_CALLBACK, acestream_load_callback, p_ace );
        p2p_SetCallback( p_p2p, P2P_PLAY_CALLBACK, acestream_play_callback, p_ace );
        p2p_SetCallback( p_p2p, P2P_STOP_CALLBACK, acestream_stop_callback, p_ace );
        p2p_SetCallback( p_p2p, P2P_CANSAVE_CALLBACK, acestream_cansave_callback, p_ace );
    }
}

static void unbind_callbacks( libvlc_acestream_object_t *p_ace )
{
    p2p_object_t *p_p2p = getP2P( p_ace->p_libvlc_instance );
    if( p_p2p ) {
        var_DelCallback( p_p2p, "auth", acestream_auth_changed, p_ace );
        var_DelCallback( p_p2p, "state", acestream_state_changed, p_ace );
        var_DelCallback( p_p2p, "info", acestream_info_changed, p_ace );
        var_DelCallback( p_p2p, "status", acestream_status_changed, p_ace );
        var_DelCallback( p_p2p, "status-raw", acestream_status_raw_changed, p_ace );
        var_DelCallback( p_p2p, "error", acestream_error_changed, p_ace );
        var_DelCallback( p_p2p, "show-userdata-dialog", acestream_showuserdata_dialog, p_ace );
        var_DelCallback( p_p2p, "show-playlist", acestream_showplaylist, p_ace );
        var_DelCallback( p_p2p, "showdialog", acestream_showerror_dialog, p_ace );
        var_DelCallback( p_p2p, "adparams", acestream_adparams, p_ace );
        var_DelCallback( p_p2p, "exit-fullscreen", acestream_exit_fullscreen, p_ace );
        var_DelCallback( p_p2p, "load-url", acestream_loadurl, p_ace );
        var_DelCallback( p_p2p, "showinfowindow", acestream_showinfowindow, p_ace );
    }
}

static void emit_events(libvlc_acestream_object_t *p_ace )
{
    p2p_object_t *p_p2p = getP2P( p_ace->p_libvlc_instance );
    if(p_p2p) {
        int auth = var_GetInteger(p_p2p, "auth");
        libvlc_event_t event_auth;
        event_auth.type = libvlc_AcestreamAuth;
        event_auth.u.acestream_auth.auth = auth;
        libvlc_event_send( p_ace->p_event_manager, &event_auth );
    
        int state = var_GetInteger(p_p2p, "state");
        libvlc_event_t event_state;
        event_state.type = libvlc_AcestreamState;
        event_state.u.acestream_state.state = state;
        libvlc_event_send( p_ace->p_event_manager, &event_state );

        if(!p_ace->b_preplay_got) {
            msg_P2PLog( p_p2p, "[emit_events] Requesting preplay load url" );    
            p2p_RequestLoadUrlAd( p_p2p, libvlc_ace_loadurl_Preplay, 0 );
        }
    }
}

libvlc_acestream_object_t *libvlc_acestream_object_new( libvlc_instance_t *p_instance )
{
    libvlc_acestream_object_t *p_ace;
    p_ace = calloc( 1, sizeof(libvlc_acestream_object_t) );
    if( unlikely(p_ace == NULL) ) {
        libvlc_printerr( "Not enough memory" );
        return NULL;
    }
    p_ace->p_event_manager = libvlc_event_manager_new( p_ace, p_instance );
    if( unlikely(p_ace->p_event_manager == NULL) ) {
        free (p_ace);
        return NULL;
    }

    libvlc_retain(p_instance);
    p_ace->p_mlist_player = NULL;
    p_ace->p_libvlc_instance = p_instance;
    p_ace->i_refcount = 1;
    p_ace->b_preplay_got = false;
    
    vlc_mutex_init( &p_ace->object_lock );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamAuth );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamState );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamInfo );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamStatus );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamStatusRaw );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamError );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamShowUserDataDialog );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamShowPlaylist );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamShowErrorDialog );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamForceExitFullscreen );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamAdParams );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamLoadUrl );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamClearLoadUrl );
    libvlc_event_manager_register_event_type( p_ace->p_event_manager, libvlc_AcestreamShowInfoWindow );
    bind_callbacks( p_ace );

    return p_ace;
}

void libvlc_acestream_object_release( libvlc_acestream_object_t *p_ace )
{
    if( !p_ace )
        return;
        
    vlc_mutex_lock( &p_ace->object_lock );
    p_ace->i_refcount--;
    if( p_ace->i_refcount > 0 ) {
        vlc_mutex_unlock( &p_ace->object_lock );
        return;
    }

    assert( p_ace->i_refcount == 0 );
    
    unbind_callbacks( p_ace );
    
    if( p_ace->p_mlist_player ) {
        libvlc_media_list_player_release( p_ace->p_mlist_player );
    }

    vlc_mutex_unlock( &p_ace->object_lock );
    vlc_mutex_destroy( &p_ace->object_lock );

    libvlc_event_manager_release( p_ace->p_event_manager );

#ifndef _WIN32
    deactivateP2P(p_ace->p_libvlc_instance);
#endif
    
    libvlc_release( p_ace->p_libvlc_instance );
    free( p_ace );
}

void libvlc_acestream_object_retain( libvlc_acestream_object_t *p_ace )
{
    if( !p_ace )
        return;

    vlc_mutex_lock( &p_ace->object_lock );
    p_ace->i_refcount++;
    vlc_mutex_unlock( &p_ace->object_lock );
}

libvlc_event_manager_t *libvlc_acestream_object_event_manager( libvlc_acestream_object_t *p_ace )
{
    return p_ace->p_event_manager;
}

void libvlc_acestream_object_ready( libvlc_acestream_object_t *p_ace )
{
    emit_events(p_ace);
}

void libvlc_acestream_object_set_media_list_player( libvlc_acestream_object_t *p_ace, libvlc_media_list_player_t *p_mlist_player )
{
    vlc_mutex_lock( &p_ace->object_lock );

    if( p_ace->p_mlist_player ) {
        libvlc_media_list_player_release( p_ace->p_mlist_player ) ;
    }
    libvlc_media_list_player_retain( p_mlist_player );
    p_ace->p_mlist_player = p_mlist_player;

    vlc_mutex_unlock( &p_ace->object_lock );
}

int libvlc_acestream_object_load( libvlc_acestream_object_t *p_ace, const char *psz_id, const char *psz_name, libvlc_acestream_id_type_t type,
                                                libvlc_acestream_load_type_t load_type, const char *option_string, int developer, int affiliate, int zone )
{
    int ret = -1;
    vlc_mutex_lock( &p_ace->object_lock );
    
    ret = p2p_LoadWithOptString( getP2P( p_ace->p_libvlc_instance ), psz_id, psz_name, type, load_type, option_string, 0, 0, NULL, true, false, developer, affiliate, zone, load_type == libvlc_ace_load_Sync );
    
    vlc_mutex_unlock( &p_ace->object_lock );
    return ret;
}

bool libvlc_acestream_object_start( libvlc_acestream_object_t *p_ace, const char *psz_id, const char *psz_indexes, libvlc_acestream_id_type_t type,
                                                int quality, int developer, int affiliate, int zone, int start_position )
{
    bool ret = false;
    vlc_mutex_lock( &p_ace->object_lock );
    
    ret = p2p_Start( getP2P( p_ace->p_libvlc_instance ), psz_id, psz_indexes, type, quality, developer, affiliate, zone, start_position );
    
    vlc_mutex_unlock( &p_ace->object_lock );
    return ret;
}

bool libvlc_acestream_object_stop( libvlc_acestream_object_t *p_ace )
{
    bool ret = false;
    vlc_mutex_lock( &p_ace->object_lock );
    
    ret = p2p_Stop( getP2P( p_ace->p_libvlc_instance ) );
    libvlc_media_list_player_set_fullstopped( p_ace->p_mlist_player );
    
    vlc_mutex_unlock( &p_ace->object_lock );
    return ret;
}

char *libvlc_acestream_object_get_content_id( libvlc_acestream_object_t *p_ace, const char *infohash, const char *checksum, int developer, int affiliate, int zone, char *out )
{
    char *psz = NULL;
    vlc_mutex_lock( &p_ace->object_lock );
    
    if( strcmp(checksum,"") ) 
        psz = p2p_GetCid( getP2P( p_ace->p_libvlc_instance ), infohash, checksum, developer, affiliate, zone );
    else
        psz = p2p_GetPid( getP2P( p_ace->p_libvlc_instance ), infohash, developer, affiliate, zone );
    
    vlc_mutex_unlock( &p_ace->object_lock );
    
    if(psz) {
        strcpy(out, psz);
        free(psz);
    }
    
    return out;
}

char *libvlc_acestream_object_get_content_id_by_index( libvlc_acestream_object_t *p_ace, int index, char *out )
{
    vlc_mutex_lock( &p_ace->object_lock );
    libvlc_media_list_t *p_mlist = libvlc_media_list_player_get_media_list( p_ace->p_mlist_player );
     
    libvlc_media_list_lock( p_mlist );
    libvlc_media_t *p_media = libvlc_media_list_item_at_index( p_mlist, index );
    libvlc_media_list_unlock( p_mlist );
    
    if( p_media && libvlc_media_is_acestream_media(p_media) ) {
        if( libvlc_media_get_acestream_url_type(p_media) == libvlc_ace_id_type_Player ) {
            libvlc_media_get_acestream_url( p_media, out );
        }
        else {
            char *psz_infohash = input_item_GetP2PInfohash( p_media->p_input_item );
            if( psz_infohash ) {
                int dev = input_item_GetP2PDeveloper( p_media->p_input_item );
                int aff = input_item_GetP2PAffiliate( p_media->p_input_item );
                int zone = input_item_GetP2PZone( p_media->p_input_item );
                char *psz_checksum = input_item_GetP2PChecksum( p_media->p_input_item );
                
                if( psz_checksum ) {
                    char *psz = p2p_GetCid( getP2P( p_ace->p_libvlc_instance ), psz_infohash, psz_checksum, dev, aff, zone );
                    if(psz) {
                        strcpy(out, psz);
                        free(psz);
                    }
                    
                    free(psz_checksum);
                }
                else {
                    char *psz = p2p_GetPid( getP2P( p_ace->p_libvlc_instance ), psz_infohash, dev, aff, zone );
                    if(psz) {
                        strcpy(out, psz);
                        free(psz);
                    }
                }
                free(psz_infohash);
            }
        }
    }
    if( p_media )
        libvlc_media_release(p_media);

    vlc_mutex_unlock( &p_ace->object_lock );
    return out;
}

bool libvlc_acestream_object_save( libvlc_acestream_object_t *p_ace, const char *infohash, int item_index, const char *pathtofile )
{
    bool ret = false;
    vlc_mutex_lock( &p_ace->object_lock );
    
    ret = p2p_Save( getP2P( p_ace->p_libvlc_instance ), infohash, item_index, pathtofile );
    
    vlc_mutex_unlock( &p_ace->object_lock );
    return ret;
}

bool libvlc_acestream_object_live_seek( libvlc_acestream_object_t *p_ace, int position )
{
    bool ret = false;
    vlc_mutex_lock( &p_ace->object_lock );
    
    ret = p2p_LiveSeek( getP2P( p_ace->p_libvlc_instance ), position );
    
    vlc_mutex_unlock( &p_ace->object_lock );
    return ret;
}

bool libvlc_acestream_object_user_data( libvlc_acestream_object_t *p_ace, int gender, int age )
{
    bool ret = false;
    vlc_mutex_lock( &p_ace->object_lock );
    
    ret = p2p_UserData( getP2P( p_ace->p_libvlc_instance ), gender, age );
    
    vlc_mutex_unlock( &p_ace->object_lock );
    return ret;
}

bool libvlc_acestream_object_user_data_mining( libvlc_acestream_object_t *p_ace, int value )
{
    bool ret = false;
    vlc_mutex_lock( &p_ace->object_lock );
    
    ret = p2p_UserDataMining( getP2P( p_ace->p_libvlc_instance ), value );
    
    vlc_mutex_unlock( &p_ace->object_lock );
    return ret;
}

bool libvlc_acestream_object_infowindow_response( libvlc_acestream_object_t *p_ace, const char *type, int button )
{
    bool ret = false;
    vlc_mutex_lock( &p_ace->object_lock );    
    ret = p2p_InfoWindowsResponse( getP2P( p_ace->p_libvlc_instance ), type, button );
    vlc_mutex_unlock( &p_ace->object_lock );
    return ret;
}

char *libvlc_acestream_object_get_engine_version( libvlc_acestream_object_t *p_ace, char *out )
{
    char *psz = NULL;
    vlc_mutex_lock( &p_ace->object_lock );
    
    psz = var_GetString( getP2P( p_ace->p_libvlc_instance ), "engine-version" );
    
    vlc_mutex_unlock( &p_ace->object_lock );
    
    if(psz) {
        strcpy(out, psz);
        free(psz);
    }

    return out;
}

void libvlc_acestream_object_activate_video_click( libvlc_acestream_object_t *p_ace, bool val )
{
    vlc_mutex_lock( &p_ace->object_lock );
    p2p_VideoClickActivate( getP2P( p_ace->p_libvlc_instance ), val );
    vlc_mutex_unlock( &p_ace->object_lock );
}

void libvlc_acestream_object_skip( libvlc_acestream_object_t *p_ace )
{
    vlc_mutex_lock( &p_ace->object_lock );
    libvlc_media_player_t *p_mi = libvlc_media_list_player_get_media_player( p_ace->p_mlist_player );
    libvlc_media_t *p_md = libvlc_media_player_get_media( p_mi );
        
    if( p_md ) {
        char *url = libvlc_media_get_mrl(p_md);
        p2p_Skip( getP2P( p_ace->p_libvlc_instance ), url );
        free(url);
        libvlc_media_release(p_md);
    }
    vlc_mutex_unlock( &p_ace->object_lock );
}

void libvlc_acestream_object_request_loadurl(libvlc_acestream_object_t *p_ace, libvlc_acestream_loadurl_type_t type)
{
    vlc_mutex_lock( &p_ace->object_lock );
    p2p_RequestLoadUrlAd( getP2P(p_ace->p_libvlc_instance), type, 0 );
    vlc_mutex_unlock( &p_ace->object_lock );
}

void libvlc_acestream_object_request_loadurl_from_group(libvlc_acestream_object_t *p_ace, libvlc_acestream_loadurl_type_t type, int group_id)
{
    vlc_mutex_lock( &p_ace->object_lock );
    p2p_RequestLoadUrlAd( getP2P(p_ace->p_libvlc_instance), type, group_id );
    vlc_mutex_unlock( &p_ace->object_lock );
}

void libvlc_acestream_object_register_loadurl_statistics(libvlc_acestream_object_t *p_ace, 
                                            libvlc_acestream_loadurl_type_t type,
                                            libvlc_acestream_loadurl_event_type_t event_type,
                                            const char *id)
{
    vlc_mutex_lock( &p_ace->object_lock );
    p2p_RegisterLoadUrlAdStatistics( getP2P(p_ace->p_libvlc_instance), type, event_type, id );
    vlc_mutex_unlock( &p_ace->object_lock );
}

void libvlc_acestream_object_register_loadurl_event(libvlc_acestream_object_t *p_ace, 
                                            libvlc_acestream_loadurl_type_t type,
                                            const char *event_type,
                                            const char *id )
{
    vlc_mutex_lock( &p_ace->object_lock );
    p2p_RegisterLoadUrlAdEvent( getP2P(p_ace->p_libvlc_instance), type, event_type, id );
    vlc_mutex_unlock( &p_ace->object_lock );
}

char *libvlc_acestream_object_get_engine_http_host( libvlc_acestream_object_t *p_ace, char *out )
{
    char *host = NULL;
    vlc_mutex_lock( &p_ace->object_lock );
    host = var_GetString( getP2P( p_ace->p_libvlc_instance ), "engine-http-host" );
    vlc_mutex_unlock( &p_ace->object_lock );
    if(host) {
        strcpy(out, host);
        free(host);
    }
    return out;
}

int libvlc_acestream_object_get_engine_http_port( libvlc_acestream_object_t *p_ace )
{
    int port = 0;
    vlc_mutex_lock( &p_ace->object_lock );    
    port = var_GetInteger( getP2P( p_ace->p_libvlc_instance ), "engine-http-port" );
    vlc_mutex_unlock( &p_ace->object_lock );
    return port;
}

void libvlc_acestream_object_restart_last(libvlc_acestream_object_t *p_ace)
{
    vlc_mutex_lock( &p_ace->object_lock );
    p2p_RestartLast( getP2P(p_ace->p_libvlc_instance) );
    vlc_mutex_unlock( &p_ace->object_lock );
}