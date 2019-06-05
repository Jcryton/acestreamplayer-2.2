#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <p2p_object.h>
#include <sstream>
#include <time.h>

#include "out.h"

#include <assert.h>

using namespace std;

inline char get_last_char(string &s)
{
    if (s.length() == 0) 
        return '\0';
    return s[s.length()-1];
}

string Out::Build( base_out_message *msg )
{
    string _msg_str = "";
    
    assert( msg->type > OUT_MSG_UNDF && msg->type < OUT_MSG_MAX );
    
    switch( msg->type ) {
    case OUT_MSG_HELLO :
        _msg_str = Out::hello( static_cast<hello_out_msg *>(msg) );
        break;
    case OUT_MSG_READY :
        _msg_str = Out::ready( static_cast<ready_out_msg *>(msg) );
        break;
    case OUT_MSG_LOAD :
        _msg_str = Out::load( static_cast<load_out_msg *>(msg) );
        break;
    case OUT_MSG_LOAD_ASYNC :
        _msg_str = Out::load_async( static_cast<load_async_out_msg *>(msg) );
        break;
    case OUT_MSG_START :
        _msg_str = Out::start( static_cast<start_out_msg *>(msg) );
        break;
    case OUT_MSG_STOP :
        _msg_str = Out::stop( static_cast<stop_out_msg *>(msg) );
        break;
    case OUT_MSG_DURATION :
        _msg_str = Out::duration( static_cast<duration_out_msg *>(msg) );
        break;
    case OUT_MSG_PLAYBACK : 
        _msg_str = Out::playback( static_cast<playback_out_msg *>(msg) );
        break;
    case OUT_MSG_SAVE :
        _msg_str = Out::save( static_cast<save_out_msg *>(msg) );
        break;
    case OUT_MSG_GET_PID :
        _msg_str = Out::get_pid( static_cast<get_pid_out_msg *>(msg) );
        break;
    case OUT_MSG_GET_CID :
        _msg_str = Out::get_cid( static_cast<get_cid_out_msg *>(msg) );
        break;
    case OUT_MSG_GET_AD_URL :
        _msg_str = Out::get_ad_url( static_cast<get_ad_url_out_msg *>(msg) );
        break;
    case OUT_MSG_LIVE_SEEK :
        _msg_str = Out::live_seek( static_cast<live_seek_out_msg *>(msg) );
        break; 
    case OUT_MSG_USER_DATA :
        _msg_str = Out::user_data( static_cast<user_data_out_msg *>(msg) );
        break;
    case OUT_MSG_SHUTDOWN :
        _msg_str = Out::shutdown( static_cast<shutdown_out_msg *>(msg) );
        break;
    case OUT_MSG_STAT_EVENT :
        _msg_str = Out::stat_event( static_cast<stat_event_out_msg *>(msg) );
        break;
    case OUT_MSG_LOAD_URL_EVENT :
        _msg_str = Out::load_url_event( static_cast<load_url_event_out_msg *>(msg) );
        break;
    case OUT_MSG_USER_DATA_MINING :
        _msg_str = Out::user_data_mining( static_cast<user_data_mining_out_msg *>(msg) );
        break;
    case OUT_MSG_INFOWND_RESPONSE :
        _msg_str = Out::infowindow_response( static_cast<infowindow_response_out_msg *>(msg) );
        break;
    default:
        break;
    }
    return _msg_str;
}

string Out::hello( hello_out_msg *msg )
{
    stringstream _msg;
    _msg << "HELLOBG" << " version=" << msg->version << " ace=" << msg->client_version << " ie=" << msg->ie_version;
    return _msg.str();
}

string Out::ready( ready_out_msg *msg )
{
    stringstream _msg;
    _msg << "READY";
    if(msg->key != "") {
        _msg << " key=" << msg->key;
    }
    return _msg.str();
}

string Out::load( load_out_msg *msg )
{
    stringstream _msg;
    _msg << "LOAD" << " ";
    switch( msg->content_type ) {
    case P2P_TYPE_TORRENT_URL :
        _msg << "TORRENT";
        break;
    case P2P_TYPE_INFOHASH :
        _msg << "INFOHASH";
        break;
    case P2P_TYPE_PLAYER :
         _msg << "PID";
        break;
    case P2P_TYPE_RAW :
        _msg << "RAW";
        break;
    default:
        return string("");
    }
    _msg << " " << msg->id;
    
    if( msg->content_type != P2P_TYPE_PLAYER ) {
        _msg << " " << msg->developer_id;
        _msg << " " << msg->affiliate_id;
        _msg << " " << msg->zone_id;
    }
    return _msg.str();
}

string Out::load_async( load_async_out_msg *msg )
{
    stringstream _msg;
    _msg << "LOADASYNC" << " " << msg->load_id << " ";
    switch( msg->content_type ) {
    case P2P_TYPE_TORRENT_URL :
        _msg << "TORRENT";
        break;
    case P2P_TYPE_INFOHASH :
        _msg << "INFOHASH";
        break;
    case P2P_TYPE_PLAYER :
         _msg << "PID";
        break;
    case P2P_TYPE_RAW :
        _msg << "RAW";
        break;
    default:
        return string("");
    }
    _msg << " " << msg->id;
    
    if( msg->content_type != P2P_TYPE_PLAYER ) {
        _msg << " " << msg->developer_id;
        _msg << " " << msg->affiliate_id;
        _msg << " " << msg->zone_id;
    }
    return _msg.str();
}

string Out::start( start_out_msg *msg )
{
    stringstream _msg;
    _msg << "START" << " ";
    switch( msg->content_type ) {
    case P2P_TYPE_TORRENT_URL :
        _msg << "TORRENT";
        break;
    case P2P_TYPE_DIRECT_URL:
        _msg << "URL";
        break;
    case P2P_TYPE_INFOHASH :
        _msg << "INFOHASH";
        break;
    case P2P_TYPE_PLAYER :
         _msg << "PID";
        break;
    case P2P_TYPE_RAW :
        _msg << "RAW";
        break;
    case P2P_TYPE_EFILE :
        _msg << "EFILE";
        break;
    default:
        return string("");
    }
    _msg << " " << msg->id;
    
    if( msg->content_type != P2P_TYPE_DIRECT_URL && msg->content_type != P2P_TYPE_EFILE && msg->indexes != "" )
        _msg << " " << msg->indexes;

    if( msg->content_type != P2P_TYPE_PLAYER && msg->content_type != P2P_TYPE_EFILE ) {
        _msg << " " << msg->developer_id;
        _msg << " " << msg->affiliate_id;
        _msg << " " << msg->zone_id;
    }

    if( msg->position != 0 )
        _msg << " " << msg->position;

    if( msg->quality >= 0 )
        _msg << " " << msg->quality;

    return _msg.str();
}

string Out::stop( stop_out_msg *msg )
{
    (void)msg;
    return string( "STOP" );
}

string Out::duration( duration_out_msg *msg )
{
    stringstream _msg;
    _msg << "DUR" << " " << msg->url << " " << msg->duration;
    return _msg.str();
}

string Out::playback( playback_out_msg *msg )
{
    stringstream _msg;
    _msg << "PLAYBACK" << " " << msg->url << " " << (int)msg->playback;
    return _msg.str();
}

string Out::save( save_out_msg *msg )
{
    stringstream _msg;
    _msg << "SAVE" << " path=" << msg->save_to << " infohash=" << msg->infohash << " index=" << msg->index;
    return _msg.str();
}

string Out::get_pid( get_pid_out_msg *msg )
{
    stringstream _msg;
    _msg << "GETPID" << " " << 
        msg->infohash << " " << 
        msg->developer_id << " " << 
        msg->affiliate_id << " " << 
        msg->zone_id;
    return _msg.str();
}

string Out::get_cid( get_cid_out_msg *msg )
{
    stringstream _msg;
    _msg << "GETCID" << " " << 
        " infohash=" << msg->infohash <<
        " checksum=" << msg->checksum <<
        " developer=" << msg->developer_id <<
        " affiliate=" << msg->affiliate_id <<
        " zone=" << msg->zone_id;
    return _msg.str();
}

string Out::get_ad_url( get_ad_url_out_msg *msg )
{
    stringstream _msg;
    _msg << "GETADURL" << " " << 
        " width=" << msg->width <<
        " height=" << msg->height <<
        " infohash=" << msg->infohash <<
        " action=" << msg->action;
    return _msg.str();
}

string Out::live_seek( live_seek_out_msg *msg )
{
    stringstream _msg;
    _msg << "LIVESEEK" << " " << msg->pos;
    return _msg.str();
}

string Out::user_data( user_data_out_msg *msg )
{
    stringstream _msg;
    _msg << "USERDATA" << " [{\"gender\": " << msg->gender << "}, {\"age\": " << msg->age << "}]";
    return _msg.str();
}

string Out::shutdown( shutdown_out_msg *msg )
{
    (void)msg;
    return string( "SHUTDOWN" );
}

string Out::stat_event( stat_event_out_msg *msg )
{
    stringstream _msg;
    
    switch( msg->stat_event_type ) {
    case P2P_STAT_PLAY :
        _msg << "EVENT play";
        break;
    case P2P_STAT_PAUSE :
        _msg << "EVENT pause position=" << msg->position;
        break;
    case P2P_STAT_SEEK :
        _msg << "EVENT seek position=" << msg->position;
        break;
    case P2P_STAT_STOP :
        _msg << "EVENT stop";
        break;
    default :
        break;
    }
    
    return _msg.str();
}

string Out::load_url_event( load_url_event_out_msg *msg )
{
    stringstream _msg;
    _msg << "EVENT " << msg->event_name << " type=" << msg->event_type << " id=" << msg->id;
    return _msg.str();
}

string Out::user_data_mining( user_data_mining_out_msg *msg )
{
    stringstream _msg;
    _msg << "USERDATA" << " [{\"mining\": " << msg->value << "}]";
    return _msg.str();
}

string Out::infowindow_response( infowindow_response_out_msg *msg )
{
    stringstream _msg;
    _msg << "EVENT infowindow_response type=" << msg->strtype << " button=" << msg->button;
    return _msg.str();
}
