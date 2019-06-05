/*****************************************************************************
 * p2p_access.cpp: p2p module
 *****************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include "control.h"
#include "p2p_access.h"
#include "api/out.h"
#include "api/in.h"

#include <time.h>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

static int Create( vlc_object_t* );
static void Destroy( vlc_object_t* );
void *Thread ( void * );

vlc_module_begin();
    set_category( CAT_ADVANCED );
    set_subcategory( SUBCAT_ADVANCED_P2P );
    add_shortcut( "p2p_access" );
    set_capability( "p2p", 0 );
#ifdef TORRENT_STREAM
    set_description( N_("Torrent Stream p2p access module") );
    set_shortname( N_("Torrent Stream p2p access") );
#else
    set_description( N_("Ace Stream p2p access module") );
    set_shortname( N_("Ace Stream p2p access") );
#endif
    set_callbacks( Create, Destroy );
    add_string("ace-host", "", N_("Remote engine server"), N_("Remote engine server"), false)
    add_integer("ace-port", 0, N_("Remote engine port"), N_("Remote engine port"), false )
    add_string("ace-developer-key", "", N_("AceStream developer key"), N_("AceStream developer key"), false)
    add_bool( "ace-get-url", false, N_("Get content url"), N_("Get content url without starting playback"), false)
vlc_module_end();

static inline int StartThread( p2p_object_t *vlc_obj )
{
    if( !var_InheritBool( vlc_obj, "no-p2p-access" ) ) {
        p2p_object_sys_t *p_sys = vlc_obj->p_sys;
        msg_P2PLog( vlc_obj, "[p2p_access.cpp::StartThread] ..." );
        if( p_sys->p_thread )
            vlc_join( p_sys->p_thread, NULL );
        p_sys->state = P2P_STATE_LAUNCHING;
        return vlc_clone( &p_sys->p_thread, Thread, vlc_obj, VLC_THREAD_PRIORITY_HIGHEST ) ? VLC_EGENERIC : VLC_SUCCESS;
    }
    else {
        msg_P2PLog( vlc_obj, "no-p2p-access option, engine not launching..." );
        return VLC_SUCCESS;
    }
}

static int Create( vlc_object_t *p_this )
{
    p2p_object_t *vlc_obj = (p2p_object_t *)p_this;
    vlc_obj->pf_load_with_opt_array = LoadTorrentWithOptArray;
    vlc_obj->pf_load_with_opt_string = LoadTorrentWithOptString;
    vlc_obj->pf_start = Start;
    vlc_obj->pf_stop = Stop;
    vlc_obj->pf_duration = Duration;
    vlc_obj->pf_playback = Playback;
    vlc_obj->pf_get_pid = GetPid;
    vlc_obj->pf_get_cid = GetCid;
    vlc_obj->pf_save = Save;
    vlc_obj->pf_get_ad_url = GetAdUrl;
    vlc_obj->pf_live_seek = LiveSeek;
    vlc_obj->pf_user_data = UserData;
    vlc_obj->pf_stat_event = StatisticsEvent;
    vlc_obj->pf_save_option = SaveOption;
    vlc_obj->pf_set_callback = SetCallback;
    vlc_obj->pf_user_data_mining = UserDataMining;
    vlc_obj->pf_register_load_url_ad_stat = RegistedLoadUrlAdStatistics;
    vlc_obj->pf_request_load_url_ad = RequestLoadUrlAd;
    vlc_obj->pf_register_load_url_ad_event = RegistedLoadUrlAdEvent;
    vlc_obj->pf_restart_last = RestartLast;
    vlc_obj->pf_info_window_response = InfoWindowResponse;
    
    srand(time(NULL)); 
    
	p2p_object_sys_t *p_sys = vlc_obj->p_sys = new p2p_object_sys_t;
    p_sys->p_thread = NULL;
    p_sys->p_control = NULL;
    p_sys->state = (p2p_state_t)var_GetInteger( vlc_obj, "state" );
    p_sys->auth = var_GetInteger( vlc_obj, "auth" );
    p_sys->will_play = false;
    
    p_sys->async_loading_items = new std::map<int, async_loading_item>;
    p_sys->out_messages_queue = new std::vector<std::string>;
    p_sys->groups_register = new std::vector<int>;
    p_sys->callbacks = new std::map<p2p_command_callback_type, std::pair<p2p_common_callback, void*> >;
    p_sys->last_start_cmd_string = "";
    
    msg_P2PLog( vlc_obj, "Starting p2p access module" );
    
    if( StartThread( vlc_obj ) != VLC_SUCCESS ) {
        msg_P2PLog( vlc_obj, "Cannot lauch p2p access thread" );
        msg_Err( vlc_obj, "Cannot lauch p2p access thread" );
        delete p_sys;
        return VLC_EGENERIC;
    }

    return VLC_SUCCESS;
}

static void Destroy( vlc_object_t *p_this )
{
    p2p_object_t *vlc_obj = (p2p_object_t *)p_this;
    p2p_object_sys_t *p_sys = vlc_obj->p_sys;

    if( !p_sys->callbacks->empty() )
        p_sys->callbacks->clear();
    delete p_sys->callbacks;
    p_sys->callbacks = NULL;
    
    if( !p_sys->async_loading_items->empty() )
        p_sys->async_loading_items->clear();
    delete p_sys->async_loading_items;
    p_sys->async_loading_items = NULL;

    if( !p_sys->out_messages_queue->empty() )
        p_sys->out_messages_queue->clear();
    delete p_sys->out_messages_queue;
    p_sys->out_messages_queue = NULL;

    if( !p_sys->groups_register->empty() )
        p_sys->groups_register->clear();
    delete p_sys->groups_register;
    p_sys->groups_register = NULL;

    if( p_sys->p_control )
        p_sys->p_control->shutdown();

    if( p_sys->p_thread ) {
        msg_P2PLog( vlc_obj, "waiting for p2p access thread..." );
        vlc_join (p_sys->p_thread, NULL);
    }
    delete p_sys;
    msg_Dbg( vlc_obj, "p2p access module stopped" );
}

void *Thread ( void *p_data )
{
    p2p_object_t *vlc_obj = (p2p_object_t *)p_data;
    p2p_object_sys_t *p_sys = vlc_obj->p_sys;

    Control control( vlc_obj );
    p_sys->p_control = &control;
    
    msg_P2PLog(vlc_obj, "[p2p_access.cpp::Thread] Starting loop");
    
    std::string _rawEngineCmd, _engineCmd;
    bool _lastCmdInRaw = false;
    size_t _delimPos;
    
    while( !control.isShutdown() ) {
        if( !control.isReady() ) {
            p_sys->state = P2P_STATE_CONNECTING;
            var_SetInteger( vlc_obj, "state", P2P_STATE_CONNECTING );
            var_SetString( vlc_obj, "info", "" );
            var_SetString( vlc_obj, "status", _("Loading...") );
            
            if( !control.startup() ) {
                p_sys->state = P2P_STATE_NOTLAUNCHED;
                var_SetInteger( vlc_obj, "state", P2P_STATE_NOTLAUNCHED );
                msg_Err( vlc_obj, "Cannot start p2p_access thread" );
                
                set_message_for_error_dialog( vlc_obj, std::string(_("Error")), _("Cannot connect to engine. Check if engine installed.") );
                msg_P2PLog(vlc_obj, "[p2p_access.cpp::Thread] Exiting loop");
                p_sys->p_control = NULL;
                return NULL;
            }
            var_SetString( vlc_obj, "status", "" );
            control.ready();
        }
        
        if( !control.receive( _rawEngineCmd ) ) {
            msg_P2PLog(vlc_obj, "[p2p_access.cpp::Thread]: Unable to receive the command from engine");
        }
        
        _lastCmdInRaw = false;
        while( !_lastCmdInRaw ) {
            _delimPos = _rawEngineCmd.find( "\r\n" );
            
            if( _delimPos == std::string::npos ) {
                _engineCmd = _rawEngineCmd;
                _lastCmdInRaw = true;
            }
            else {
                _engineCmd = _rawEngineCmd.substr( 0, _delimPos );
                _rawEngineCmd.erase( 0, _delimPos + 2 );
                if( _rawEngineCmd.size() == 0 )
                    _lastCmdInRaw = true;
            }

            base_in_message *in_msg = In::Parse( _engineCmd );
            if( in_msg->type == IN_MSG_LOAD ) {
                load_in_msg *load = static_cast<load_in_msg *>(in_msg);

                if( p_sys->async_loading_items ) {
                    std::map<int, async_loading_item>::iterator it = p_sys->async_loading_items->find(load->load_id);
                    if( it == p_sys->async_loading_items->end() ){
                        msg_Warn( vlc_obj, "got unexpected async id %d", load->load_id );
                        msg_P2PLog(vlc_obj, "[p2p_access.cpp::Thread] LOAD EVENT with unexpected load_id %d", load->load_id);
                    }
                    else {
                        async_loading_item async_load_item = it->second;
                        p_sys->async_loading_items->erase(it);
                        
                        if( check_load_response( vlc_obj, load, async_load_item.id ) )
                            control.processEngineLoadMessage( load, async_load_item );
                    }
                }
            }
            else if( in_msg->type == IN_MSG_LOAD_URL ) {
                control.processEngineMessage( in_msg );
                if( p_sys->out_messages_queue && p_sys->out_messages_queue->size() > 0 ) {
                    msg_P2PLog(vlc_obj, "[p2p_access.cpp::Thread]: Send commands from queue...");
                    for( std::vector<std::string>::iterator it = p_sys->out_messages_queue->begin(); it != p_sys->out_messages_queue->end(); ++it ) {
                        control.sendString( *it );
                    }
                    p_sys->out_messages_queue->clear();
                }
            }
            else if( in_msg->type == IN_MSG_PLAY) {
                bool get_url_only = var_CreateGetBool(vlc_obj, "ace-get-url");
                play_in_msg *play = static_cast<play_in_msg *>(in_msg);
                if(get_url_only && play->play_type != P2P_PLAY_INTERRUPTABLE_AD && play->play_type != P2P_PLAY_AD) {
                    msg_Info(vlc_obj, "[p2p_access.cpp::Thread]: Got play command. Url: %s", play->url.c_str());
                    set_message_for_error_dialog(vlc_obj, std::string("URL"), "%s", play->url.c_str());
                }
                else {
                    control.processEngineMessage(in_msg);
                }
            }
            else {
                control.processEngineMessage( in_msg );
            }
            delete in_msg;
        }
    }
   
    msg_P2PLog(vlc_obj, "[p2p_access.cpp::Thread] Exiting loop");
    p_sys->p_control = NULL;
    return NULL;
}

int LoadTorrentWithOptArray( p2p_object_t *vlc_obj, const char *id, 
            const char *name, p2p_uri_id_type_t type, bool async,
            int optcount, const char* const *optarr, int add_mode, int callback_type, playlist_item_t *p_parent_node,
            bool playlist, bool start,
            int developer, int affiliate, int zone, bool object_locked )
{
    std::string optstring = "";
    for( size_t _i = 0; _i < optcount; ++_i )
        optstring.append(std::string(optarr[_i])).append(" ");

    if( optstring != "" )
        optstring.erase( optstring.length()-1 );

    return LoadTorrentWithOptString( vlc_obj, id, name, type, async, optstring.length() > 0 ? optstring.c_str() : NULL, add_mode, callback_type, p_parent_node, playlist, start, developer, affiliate, zone, object_locked );
}

int LoadTorrentWithOptString( p2p_object_t *vlc_obj, const char *id, 
            const char *name, p2p_uri_id_type_t type, bool async,
            const char *optstring, int add_mode, int callback_type, playlist_item_t *p_parent_node, 
            bool playlist, bool start,
            int developer, int affiliate, int zone, bool object_locked )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED ) {
        if( StartThread( vlc_obj ) != VLC_SUCCESS )
            return VLC_EGENERIC;
    }
    
    bool _async = async;
    bool _enqueue = false;

    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING ) {
        _async = true;
        _enqueue = true;
    }
    
    async_loading_item _async_item;
    _async_item.id = std::string( id );
    _async_item.name = name ? std::string( name ) : "";
    _async_item.type = type;
    _async_item.playlist = playlist;
    _async_item.add_mode = add_mode;
    _async_item.add_type = callback_type;
    _async_item.parent = p_parent_node;
    _async_item.developer = developer;
    _async_item.affiliate = affiliate;
    _async_item.zone = zone;
    _async_item.locked = _async ? false : object_locked;

    if( optstring ) {
        std::string _options = optstring;
        std::istringstream _iss( _options );
        std::copy(std::istream_iterator<std::string>( _iss ), std::istream_iterator<std::string>(), std::back_inserter<std::vector<std::string> >(_async_item.options));
    }
    
    if( type == P2P_TYPE_EFILE || type == P2P_TYPE_DIRECT_URL ) {
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::LoadTorrentWithOptString] EFILE or DIRECT_URL loading");
        p_sys->will_play = start;
        return p_sys->p_control ? p_sys->p_control->processEngineLoadMessage( NULL, _async_item ) : VLC_EGENERIC;
    }
    
    if( _async ) {
        int _async_id = generate_new_async_id( vlc_obj );

        load_async_out_msg _msg;
        _msg.load_id = _async_id;
        _msg.content_type = type;
        std::string id_str = std::string( id );
        _msg.id = ( type == P2P_TYPE_PLAYER && id_str.find("acestream://") == 0 ) ? id_str.substr(12, 40) : id_str;
        _msg.developer_id = developer;
        _msg.affiliate_id = affiliate;
        _msg.zone_id = zone;
        
        if( !p_sys->async_loading_items ) {
            msg_P2PLog(vlc_obj, "[p2p_access.cpp::LoadTorrentWithOptString] loading items not initialized, maybe quiting");
            return VLC_EGENERIC;
        }
        
        if( start ) {
            if( !p_sys->will_play && !p_sys->async_loading_items->size() )
                p_sys->will_play = true;
        }
        
        p_sys->async_loading_items->insert( std::pair<int, async_loading_item>(_async_id, _async_item) );
        if( _enqueue && p_sys->out_messages_queue ) {
            std::string _load_msg_str = Out::Build( &_msg );
            p_sys->out_messages_queue->push_back( _load_msg_str );
            msg_P2PLog(vlc_obj, "[p2p_access.cpp::LoadTorrentWithOptString] adding message to queue %s", _load_msg_str.c_str());
            return VLC_SUCCESS;
        }
        else {
            var_SetInteger( vlc_obj, "state", P2P_STATE_LOADING );
            
            bool _send = false;
            if( p_sys->p_control )
                _send = p_sys->p_control->send( &_msg );
            if( !_send ) {
                msg_Err( vlc_obj, "Cannot send load message to engine" );
                msg_P2PLog(vlc_obj, "[p2p_access.cpp::LoadTorrentWithOptString] sending load async error");
            }
            return _send ? VLC_SUCCESS : VLC_EGENERIC;
        }
    }
    else {
        if( !p_sys->p_control )
            return VLC_EGENERIC;
        load_out_msg _msg;
        _msg.content_type = type;
        std::string id_str = std::string( id );
        _msg.id = ( type == P2P_TYPE_PLAYER && id_str.find("acestream://") == 0 ) ? id_str.substr(12, 40) : id_str;
        _msg.developer_id = developer;
        _msg.affiliate_id = affiliate;
        _msg.zone_id = zone;
        
        p_sys->will_play = start;
        var_SetInteger( vlc_obj, "state", P2P_STATE_LOADING );
        
        base_in_message *_in_msg = p_sys->p_control->sendSync( &_msg );
        if( !_in_msg ) {
            msg_Err( vlc_obj, "Cannot load content." );
            set_message_for_error_dialog( vlc_obj, std::string(_("Error")), _("Engine could not load \"%s\""), id );
            msg_P2PLog(vlc_obj, "[p2p_access.cpp::LoadTorrentWithOptString] SYNC load return NULL");
            return VLC_EGENERIC;
        }
        
        if( _in_msg->type != IN_MSG_LOAD ) {
            msg_Err( vlc_obj, "Engine return incorrect response type" );
            set_message_for_error_dialog( vlc_obj, std::string(_("Error")), _("Engine could not load \"%s\""), id );
            msg_P2PLog(vlc_obj, "[p2p_access.cpp::LoadTorrentWithOptString] SYNC load return incorrect response message type %d", _in_msg->type);
            delete _in_msg;
            return VLC_EGENERIC;
        }
        load_in_msg *_load = static_cast<load_in_msg *>(_in_msg);
        
        if( check_load_response( vlc_obj, _load, _msg.id ) ) {
            int ret = p_sys->p_control->processEngineLoadMessage( _load, _async_item );
            delete _in_msg;
            return ret;
        }
        else {
            delete _in_msg;
            return VLC_EGENERIC;
        }
    }
}

bool Start( p2p_object_t *vlc_obj, const char *id, const char *indexes, p2p_uri_id_type_t type, int quality, 
            int developer, int affiliate, int zone, int position )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED ) {
        if( StartThread( vlc_obj ) != VLC_SUCCESS )
            return false;
    }
    
    bool _enqueue = p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING;
    
    start_out_msg _msg;
    _msg.content_type = type;
    std::string id_str = std::string( id );
    _msg.id = ( type == P2P_TYPE_PLAYER && id_str.find("acestream://") == 0 ) ? id_str.substr(12, 40) : id_str;
    _msg.indexes = indexes ? std::string( indexes ) : "";
    _msg.developer_id = developer;
    _msg.affiliate_id = affiliate;
    _msg.zone_id = zone;
    _msg.quality = quality;
    _msg.position = position;
    
    if( p_sys->p_control ) {
        p_sys->p_control->clearLoadUrl();
    }
    std::string _start_msg_str = Out::Build( &_msg );
    p_sys->last_start_cmd_string = _start_msg_str;
    if( _enqueue && p_sys->out_messages_queue ) {
        p_sys->out_messages_queue->push_back( _start_msg_str );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::LoadTorrentWithOptString] adding message to queue %s", _start_msg_str.c_str());
        return true;
    }
    else {
        bool _send = false;
        if( p_sys->p_control ) {
            var_SetAddress( vlc_obj, "preload-pause-url", NULL );
            var_SetAddress( vlc_obj, "preload-nonlinear-url", NULL );
            var_SetAddress( vlc_obj, "preload-stop-url", NULL );
            _send = p_sys->p_control->send( &_msg );
        }
        if( !_send ) {
            msg_Err( vlc_obj, "Cannot send start message to engine" );
            msg_P2PLog(vlc_obj, "[p2p_access.cpp::Start] sending start async error");
        }
        return _send;
    }
}

bool Stop( p2p_object_t *vlc_obj )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING )
        return false;
    if( !p_sys->p_control )
        return false;

    stop_out_msg _msg;
    bool _send = p_sys->p_control->send( &_msg );
    if( !_send ) {
        msg_Err( vlc_obj, "Cannot send stop message to engine" );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::Stop] sending stop async error");
    }
    return _send;
}

bool Duration( p2p_object_t *vlc_obj, const char *url, int duration )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING )
        return false;
    if( !p_sys->p_control )
        return false;
        
    duration_out_msg _msg;
    _msg.url = std::string(url);
    _msg.duration = duration;
    
    bool _send = p_sys->p_control->send( &_msg );
    if( !_send ) {
        msg_Err( vlc_obj, "Cannot send duration message to engine" );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::Duration] sending duration async error");
    }
    return _send;
}

bool Playback( p2p_object_t *vlc_obj, const char *url, p2p_playback_value_t playback )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING )
        return false;
    if( !p_sys->p_control )
        return false;
        
    playback_out_msg _msg;
    _msg.url = std::string(url);
    _msg.playback = playback;
    
    bool _send = p_sys->p_control->send( &_msg );
    if( !_send ) {
        msg_Err( vlc_obj, "Cannot send playback message to engine" );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::Playback] sending playback async error");
    }
    return _send;
}

char* GetPid( p2p_object_t *vlc_obj, const char *infohash, int developer, int affiliate, int zone )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING )
        return NULL;
    if( !p_sys->p_control )
        return NULL;
    
    get_pid_out_msg _msg;
    _msg.infohash = std::string(infohash);
    _msg.developer_id = developer;
    _msg.affiliate_id = affiliate;
    _msg.zone_id = zone;
    
    base_in_message *_in_msg = p_sys->p_control->sendSync( &_msg );
    if( !_in_msg ) {
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::GetPid] SYNC GetPid return NULL");
        return NULL;
    }
    if( _in_msg->type != IN_MSG_GET_PID_RESP ) {
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::GetPid] SYNC GetPid return incorrect response message type %d", _in_msg->type);
        delete _in_msg;
        return NULL;
    }
    get_pid_req_in_msg *_pid = static_cast<get_pid_req_in_msg *>(_in_msg);
    char *psz_pid = strdup(_pid->value.c_str());
    delete _in_msg;
    return psz_pid;
}

char* GetCid( p2p_object_t *vlc_obj, const char *infohash, const char *checksum, int developer, int affiliate, int zone )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING )
        return NULL;
    if( !p_sys->p_control )
        return NULL;

    get_cid_out_msg _msg;
    _msg.infohash = std::string(infohash);
    _msg.checksum = std::string(checksum);
    _msg.developer_id = developer;
    _msg.affiliate_id = affiliate;
    _msg.zone_id = zone;

    base_in_message *_in_msg = p_sys->p_control->sendSync( &_msg );
    if( !_in_msg ) {
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::GetCid] SYNC GetCid return NULL");
        return NULL;
    }
    if( _in_msg->type != IN_MSG_GET_CID_RESP ) {
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::GetCid] SYNC GetCid return incorrect response message type %d", _in_msg->type);
        delete _in_msg;
        return NULL;
    }
    get_cid_req_in_msg *_pid = static_cast<get_cid_req_in_msg *>(_in_msg);
    char *psz_cid = strdup(_pid->value.c_str());
    delete _in_msg;
    
    return psz_cid;
}

bool Save( p2p_object_t *vlc_obj, const char *infohash, int index, const char *save_to )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING )
        return false;
    if( !p_sys->p_control )
        return false;
    char *psz_save_to = encode_URI_component( save_to );
    if(!psz_save_to)
        return false;
    save_out_msg _msg;
    _msg.save_to = std::string(psz_save_to);
    _msg.infohash = std::string(infohash);
    _msg.index = index;
    free(psz_save_to);
    bool _send = p_sys->p_control->send( &_msg );
    if( !_send ) {
        msg_Err( vlc_obj, "Cannot send save to engine" );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::Save] sending save async error");
    }
    return _send;
}

bool GetAdUrl(p2p_object_t *vlc_obj, const char *infohash, const char *action, int width, int height)
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING )
        return false;
    if( !p_sys->p_control )
        return false;

    get_ad_url_out_msg _msg;
    _msg.infohash = std::string(infohash);
    _msg.action = std::string(action);
    _msg.width = width;
    _msg.height = height;
    bool _send = p_sys->p_control->send( &_msg );
    if( !_send ) {
        msg_Err( vlc_obj, "Cannot send getadurl to engine" );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::GetAdUrl] sending getadurl async error");
    }
    return _send;
}

bool LiveSeek(p2p_object_t *vlc_obj, int position)
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING )
        return false;
    if( !p_sys->p_control )
        return false;
        
    live_seek_out_msg _msg;
    _msg.pos = position;
    
    bool _send = p_sys->p_control->send( &_msg );
    if( !_send ) {
        msg_Err( vlc_obj, "Cannot send liveseek to engine" );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::LiveSeek] sending liveseek async error");
    }
    return _send;
}

bool UserData( p2p_object_t *vlc_obj, int gender, int age )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING )
        return false;
    if( !p_sys->p_control )
        return false;
    
    if( !p_sys->p_control->supportedTenAgesUserinfo() && age > 8 )
        age = age == 9 ? 3 : age == 10 ? 4 : age;
    
    user_data_out_msg _msg;
    _msg.gender = gender;
    _msg.age = age;
    
    bool _send = p_sys->p_control->send( &_msg );
    if( !_send ) {
        msg_Err( vlc_obj, "Cannot send userdata to engine" );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::UserData] sending userdata async error");
    }
    else {
        if( p_sys->last_start_cmd_string != "" )
            _send = p_sys->p_control->sendString( p_sys->last_start_cmd_string );
    }
    return _send;
}

bool UserDataMining( p2p_object_t *vlc_obj, int value )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING )
        return false;
    if( !p_sys->p_control )
        return false;
    
    user_data_mining_out_msg _msg;
    _msg.value = value;
    
    bool _send = p_sys->p_control->send( &_msg );
    if( !_send ) {
        msg_Err( vlc_obj, "Cannot send userdatamining to engine" );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::UserDataMining] sending userdatamining async error");
    }
    else {
        /*if( p_sys->last_start_cmd_string != "" ) {
            _send = p_sys->p_control->sendString( p_sys->last_start_cmd_string );
        }*/
    }
    return _send;
}

bool InfoWindowResponse( p2p_object_t *vlc_obj, const char *type, int button )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING )
        return false;
    if( !p_sys->p_control )
        return false;
    
    infowindow_response_out_msg _msg;
    _msg.strtype = std::string(type);
    _msg.button = button;
    
    bool _send = p_sys->p_control->send( &_msg );
    if( !_send ) {
        msg_Err( vlc_obj, "Cannot send infownd_response to engine" );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::InfoWindowResponse] sending infownd_response async error");
    }
    return _send;
}

bool StatisticsEvent(p2p_object_t *vlc_obj , p2p_statistics_event_type_t type, int value)
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    
    if( p_sys->state == P2P_STATE_NOTLAUNCHED || p_sys->state == P2P_STATE_CONNECTING || p_sys->state == P2P_STATE_LAUNCHING )
        return false;
    if( !p_sys->p_control )
        return false;
    
    if( !p_sys->p_control->supportedStatEvents() )
        return false;

    stat_event_out_msg _msg;
    _msg.stat_event_type = type;
    _msg.position = value;
    
    bool _send = p_sys->p_control->send( &_msg );
    if( !_send ) {
        msg_Err( vlc_obj, "Cannot send statisticsevent to engine" );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::StatisticsEvent] sending statisticsevent async error");
    }
    return _send;
}

bool SaveOption(p2p_object_t *vlc_obj, const char *infohash, const char *name, const char *value)
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    if( !p_sys->p_control )
        return false;
    
    std::string _infohash = std::string(infohash);
    std::string _name = std::string(name);
    std::string _value = std::string(value);
    
    return p_sys->p_control->dbSaveOption( _infohash, _name, _value );
}

void SetCallback(p2p_object_t *vlc_obj, p2p_command_callback_type type, p2p_common_callback pf_callback, void *cb_data)
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    if( !p_sys->callbacks )
        return;

    if( type < 0 || type >= P2P_MAX_CALLBACK )
        return;
        
    std::map<p2p_command_callback_type, std::pair<p2p_common_callback, void*> >::iterator it = p_sys->callbacks->find(type);
    if( pf_callback ) {
        if( it != p_sys->callbacks->end() )
            it->second = std::pair<p2p_common_callback, void*>(pf_callback, cb_data);
        else
            p_sys->callbacks->insert( std::pair<p2p_command_callback_type, std::pair<p2p_common_callback, void*> >(type, std::pair<p2p_common_callback, void*>(pf_callback, cb_data)) );
    }
    else if( it != p_sys->callbacks->end() ) {
        p_sys->callbacks->erase( it );
    }
}

void RequestLoadUrlAd(p2p_object_t *vlc_obj, p2p_load_url_type_t type, int group_id)
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    if( !p_sys->p_control ) return;
    p_sys->p_control->requestLoadUrl(type, group_id);
}

void RegistedLoadUrlAdStatistics(p2p_object_t *vlc_obj, p2p_load_url_type_t type, p2p_load_url_statistics_event_type_t event_type, const char *id)
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    if( !p_sys->p_control ) return;
    std::string _id = std::string(id);
    p_sys->p_control->registerLoadUrlStatistics(type, event_type, _id);
}

void RegistedLoadUrlAdEvent(p2p_object_t *vlc_obj, p2p_load_url_type_t type, const char *event_type, const char *id)
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    if( !p_sys->p_control ) return;
    std::string _id = std::string(id);
    std::string _event_type = std::string(event_type);
    p_sys->p_control->registerLoadUrlEvent(type, _event_type, _id);
}

void RestartLast( p2p_object_t *vlc_obj )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    if( p_sys->last_start_cmd_string != "" ) {
        p_sys->p_control->sendString( p_sys->last_start_cmd_string );
    }
}

int generate_new_async_id( p2p_object_t *vlc_obj )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;

    if( !p_sys->async_loading_items )
        return -1;
    
    int _async_id = rand() % 9999 + 1000;
    std::map<int, async_loading_item>::iterator it = p_sys->async_loading_items->find(_async_id);
    while( it != p_sys->async_loading_items->end() ) {
        _async_id = rand() % 9999 + 1000;
        it = p_sys->async_loading_items->find(_async_id);
    }
    return _async_id;
}

int generate_new_group_id( p2p_object_t *vlc_obj )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;

    if( !p_sys->groups_register )
        return -1;
    
    int _group_id = rand() % 9999 + 1000;
    
    std::vector<int>::iterator it;
    it = std::find( p_sys->groups_register->begin(), p_sys->groups_register->end(), _group_id );
    while( it != p_sys->groups_register->end() ) {
        _group_id = rand() % 9999 + 1000;
        it = std::find( p_sys->groups_register->begin(), p_sys->groups_register->end(), _group_id );
    }
    p_sys->groups_register->push_back( _group_id );
    return _group_id;
}

bool check_load_response( p2p_object_t *vlc_obj, load_in_msg *load, std::string id )
{
    p2p_object_sys_t* p_sys = vlc_obj->p_sys;
    if( load->status == IN_LOAD_ERROR ) {
        msg_Err( vlc_obj, load->err_message.c_str() );
        set_message_for_error_dialog( vlc_obj, std::string(_("Error")), _("Engine could not load \"%s\" : \"%s\""), id.c_str(), load->err_message.c_str() );
        set_message_for_error_dialog( vlc_obj, std::string(_("Error")), _("%s"), load->err_message.c_str() );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::check_load_response] load got error message: %s", load->err_message.c_str());
        return false;
    }
    
    /*if( load->status == IN_LOAD_MULTI && !p_sys->auth ) {
        msg_Err( vlc_obj, "Source %s contains a playlist. Activate Your account to play it.", id.c_str() );
        set_message_for_error_dialog( vlc_obj, std::string(_("Error")), _("Attention! This source \"%s\" contains a playlist.\n"
                                            "To play and activate the playlist you need to:\n"
                                            "- Register on site torrentstream.org;\n"
                                            "- Activate your account."), id.c_str() );
        msg_P2PLog(vlc_obj, "[p2p_access.cpp::LoadTorrentWithOptString] load playlist with auth = 0");
        return false;
    }*/
    return true;
}

void set_message_for_error_dialog( p2p_object_t *p_p2p, std::string title, const char *fmt, ... )
{
    char *msg;
    va_list ap;
    va_start(ap, fmt);
    if(vasprintf (&msg, fmt, ap) != -1) {
        p2p_showdialog_item_t p_item;
        p_item.text = msg;
        p_item.title = title.c_str();
        var_SetAddress( p_p2p, "showdialog", &p_item );
        free(msg);
    }
    va_end(ap);
}