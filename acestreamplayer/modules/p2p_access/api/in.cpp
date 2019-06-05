#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <p2p_object.h>
#include <vlc_url.h>
#include "in.h"
#include "jsoncpp/value.h"
#include "jsoncpp/reader.h"
#include <sstream>  
#include <time.h>

using namespace std;

base_in_message *In::Parse( const string &msg )
{
    base_in_message *_msg;

    if( !msg.compare( 0, 8, "NOTREADY" ) )
        _msg = static_cast<base_in_message *>( In::notready( msg ) );
    else if( !msg.compare( 0, 7, "HELLOTS" ) )
        _msg = static_cast<base_in_message *>( In::hello( msg ) );
    else if( !msg.compare( 0, 4, "AUTH" ) )
        _msg = static_cast<base_in_message *>( In::auth( msg ) );
    else if( !msg.compare( 0, 6, "STATUS" ) )
        _msg = static_cast<base_in_message *>( In::status( msg ) );
    else if( !msg.compare( 0, 5, "STATE" ) )
        _msg = static_cast<base_in_message *>( In::state( msg ) );
    else if( !msg.compare( 0, 4, "INFO" ) )
        _msg = static_cast<base_in_message *>( In::info( msg ) );
    else if( !msg.compare( 0, 5, "START" ) )
        _msg = static_cast<base_in_message *>( In::start( msg ) );
    else if( !msg.compare( 0, 4, "PLAY" ) )
        _msg = static_cast<base_in_message *>( In::play( msg ) );
    else if( !msg.compare( 0, 6, "PLAYAD" ) )
        _msg = static_cast<base_in_message *>( In::play_ad( msg ) );
    else if( !msg.compare( 0, 7, "PLAYADI" ) )
        _msg = static_cast<base_in_message *>( In::play_interruptable_ad( msg ) );
    else if( !msg.compare( 0, 5, "PAUSE" ) )
        _msg = static_cast<base_in_message *>( In::pause( msg ) );
    else if( !msg.compare( 0, 6, "RESUME" ) )
        _msg = static_cast<base_in_message *>( In::resume( msg ) );
    else if( !msg.compare( 0, 4, "STOP" ) )
        _msg = static_cast<base_in_message*>( In::stop( msg ) );
    else if( !msg.compare( 0, 8, "LOADRESP" ) )
        _msg = static_cast<base_in_message *>( In::load( msg ) );
    else if( !msg.compare( 0, 8, "SHUTDOWN" ) )
        _msg = static_cast<base_in_message *>( In::shutdown( msg ) );
    else if( !msg.compare( 0, 5, "EVENT" ) )
        _msg = static_cast<base_in_message*>( In::event( msg ) );
    else if( !msg.compare( 0, 8, "LOAD_URL" ) )
        _msg = static_cast<base_in_message*>( In::load_url( msg ) );
    else {
        _msg = new base_in_message;
        _msg->type = IN_MSG_UNDF;
    }

    return _msg;
}

base_in_message *In::ParseSyncLoad( const string &msg )
{
    base_in_message *_msg = static_cast<base_in_message*>( In::load( msg, true ) );
    _msg->raw = msg;
    return _msg;
}

base_in_message *In::ParseSyncGetPid( const string &msg )
{
    get_pid_req_in_msg *_msg = new get_pid_req_in_msg;
    _msg->value = string(msg);
    _msg->raw = msg;
    return static_cast<base_in_message*>(_msg);
}

base_in_message *In::ParseSyncGetCid( const string &msg )
{
    get_cid_req_in_msg *_msg = new get_cid_req_in_msg;
    _msg->value = string(msg);
    _msg->raw = msg;
    return static_cast<base_in_message*>(_msg);
}

vector<string> In::split( const string &base, char delim )
{
    vector<string> tokens;
   
    stringstream ss( base );
    string item;
    while( getline(ss, item, delim) ) 
        tokens.push_back( item );

    return tokens;
}

string In::decode_url( string text )
{
    char *decoded = (char*)malloc(text.length()+1);
    strncpy(decoded, text.c_str(), text.length());
    decoded[text.length()] = '\0';
    decode_URI(decoded);
    string decoded_str = string(_(decoded));
    free(decoded);
    return decoded_str;
}

bool In::is_numeric( string &text )
{
    istringstream iss(text);
    double d = 0.0;
    return (iss >> d) && iss.eof();
}

notready_in_msg *In::notready( const string &msg )
{
    notready_in_msg *_msg = new notready_in_msg;
    _msg->raw = msg;
    return _msg;
}

hello_in_msg *In::hello( const string &msg )
{
    hello_in_msg *_msg = new hello_in_msg;
    _msg->raw = msg;
    _msg->version = "";
    _msg->hello_key = "";
    _msg->major = _msg->minor = _msg->build = _msg->revision = 0;
    _msg->is_alpha = false;
    _msg->http_port = 0;
    
    vector<string> _options = In::split(msg, ' ');
    if( _options.size() > 0 ) {
         for( size_t i = 0; i < _options.size(); ++i ) {
            if( !_options[i].compare(0, 8, "version=") ) {
                _msg->version = _options[i].substr(8);
                
                vector<string> versionv = In::split(_msg->version, '.');
                _msg->major = atoi(versionv[0].c_str());
                _msg->minor = atoi(versionv[1].c_str());

                bool has_revision = false;
                if(versionv.size() > 2) {
                    size_t _pos = versionv[2].find("-");
                    if( _pos == string::npos ) {
                        _msg->build = atoi(versionv[2].c_str());
                    }
                    else {
                        string alpha = versionv[2].substr(_pos + 1);
                        versionv[2].erase(_pos);
                        _msg->build = atoi(versionv[2].c_str());
                        if(alpha[0] == 'a') {
                            alpha = alpha.substr(1);
                            if(is_numeric(alpha)) {
                                _msg->revision = atoi(alpha.c_str());
                                _msg->is_alpha = true;
                                has_revision = true;
                            }
                        }
                    }
                }
                
                if(!has_revision && versionv.size() > 3) {
                    _msg->revision = atoi(versionv[3].c_str());
                }
            }
            else if( !_options[i].compare(0, 4, "key=") ) {
                _msg->hello_key.assign(_options[i].substr(4));
            }
            else if( !_options[i].compare(0, 10, "http_port=") ) {
                _msg->http_port = atoi(_options[i].substr(10).c_str());
            }
        }
    }

    return _msg;
}

auth_in_msg *In::auth( const string &msg )
{
    auth_in_msg *_msg = new auth_in_msg;
    _msg->raw = msg;
    _msg->value = atoi( msg.substr(5).c_str() );
    return _msg;
}

status_in_msg *In::status( const string &msg )
{
    status_in_msg *_msg = new status_in_msg;
    _msg->raw = msg;
    _msg->value = "";
    _msg->is_error = false;
    
    string _base = msg.substr(7);
    size_t _pos = _base.find( "|" );    // main|ad - parse only main now
    if( _pos != string::npos ) {
        if( !_base.compare(0, 9, "ad:prebuf") ) {
            _msg->value.assign( _("Starting...") );
            return _msg;
        }
        _base.erase( _pos );
    }

    vector<string> _options = In::split( _base, ';');
    if( _options.size() > 0 ) {
        if( !_options[0].compare("main:dl") || !_options[0].compare("main:idle") )
            ;
        else if( !_options[0].compare("main:buf") ) {
            _msg->value.assign( _("Buffering ") );
            _msg->value.append( _options[1] ).append("%");
        }
        else if( !_options[0].compare("main:prebuf") ) {
            _msg->value.assign( _("Prebuffering ") );
            _msg->value.append( _options[1] ).append("% (").append( _("connected to ") ).append( _options[8] );
            _msg->value.append( _options[8].compare("1") ? _(" streams") : _(" stream") );
			_msg->value.append(")");
        }
        else if( !_options[0].compare("main:check") ) {
            _msg->value.assign( _("Checking ") );
			_msg->value.append( _options[1] ).append( "%" );
        }
        else if( !_options[0].compare("main:wait") )
            _msg->value.assign( _("Insufficient download speed to play without interruption") );
		else if( !_options[0].compare("main:err") ) {
            _msg->is_error = true;
			_msg->value.assign( _options[2] );
        }
        else if( !_options[0].compare("main:starting") )
            _msg->value.assign( _("Starting...") );
        else if( !_options[0].compare("main:loading") )
            _msg->value.assign( _("Loading...") );
    }
    return _msg;
}

state_in_msg *In::state( const string &msg )
{
    state_in_msg *_msg = new state_in_msg;
    _msg->raw = msg;
    _msg->value = (p2p_state_t)atoi( msg.substr(6).c_str() );
    return _msg;
}

info_in_msg *In::info( const string &msg )
{
    info_in_msg *_msg = new info_in_msg;
    _msg->raw = msg;
    _msg->value = "";
    
    string _base = msg.substr(5);
    vector<string> _options = In::split( _base, ';' );
    if( _options.size() > 0 ) {
        int _info_id = atoi( _options[0].c_str() );
        
        if( _info_id == 0 ) {
            if( _options.size() == 2 ) 
                _msg->value = _options[1];
        }
        else if( _info_id == 1) 
            _msg->value.assign( _("There are no active peers and streams at this moment") );
        else if( _info_id == 2 )
            _msg->value.assign( _("Advertising video") );
        else if( _info_id == 3 )
            _msg->value.assign( _("Main content") );
    }
    
    return _msg;
}

play_in_msg *In::start( const string &msg )
{
    play_in_msg*_msg = new play_in_msg;
    _msg->raw = msg;
    _msg->url = "";
    _msg->start_position = 0;
    _msg->play_type = P2P_PLAY_UNDF;
    _msg->clickurl = "";
    _msg->clicktext = "";
    _msg->skipoffset = "";
    _msg->noadsurl = "";
    _msg->volume = 15;
    _msg->noadstext = "";
    _msg->adinfotext = "";
    _msg->deinterlace_mode = "";
    
    string _base = msg.substr(6);
    vector<string> _options = In::split( _base, ' ');
    if( _options.size() > 0 ) {
        bool _ad = false;
        bool _int_ad = false;
        bool _stream = false;
        
        for( size_t i = 0; i < _options.size(); ++i ) {
            if( !_options[i].compare(0, 3, "ad=") )
                _ad = (bool)atoi( _options[i].substr(3, 1).c_str() );
            else if( !_options[i].compare(0, 14, "interruptable=") )
                _int_ad = (bool)atoi( _options[i].substr(14, 1).c_str() );
            else if( !_options[i].compare(0, 4, "pos=") )
                _msg->start_position = atoi( _options[i].substr(4, _options[i].length() - 4).c_str() );
            else if( !_options[i].compare(0, 7, "stream=") )
                _stream = (bool)atoi( _options[i].substr(7, 1).c_str() );
            else if( !_options[i].compare(0, 9, "clickurl=") )
                _msg->clickurl.assign(decode_url(_options[i].substr(9)));
            else if( !_options[i].compare(0, 10, "clicktext=") )
                _msg->clicktext.assign(decode_url(_options[i].substr(10)));
            else if( !_options[i].compare(0, 11, "skipoffset=") ) {
                _msg->skipoffset.assign(decode_url(_options[i].substr(11)));
                size_t per_pos = _msg->skipoffset.find("%");
                if(per_pos == string::npos && _msg->skipoffset.size() == 5) {
                    string new_skipoff = "00:" + _msg->skipoffset;
                    _msg->skipoffset.assign(new_skipoff);
                }
            }
            else if( !_options[i].compare(0, 11, "noads_link=") )
                _msg->noadsurl.assign(decode_url(_options[i].substr(11)));
            else if( !_options[i].compare(0, 4, "url=") )
                _msg->url.assign(decode_url(_options[i].substr(4)));
            else if( !_options[i].compare(0, 7, "volume=") )
                _msg->volume = atoi( _options[i].substr(7, _options[i].length() - 7).c_str() );
            else if( !_options[i].compare(0, 11, "noads_text=") )
                _msg->noadstext.assign(decode_url(_options[i].substr(11)));
            else if( !_options[i].compare(0, 13, "ad_info_text=") )
                _msg->adinfotext.assign(decode_url(_options[i].substr(13)));
            else if( !_options[i].compare(0, 19, "deinterlacing_mode=") )
                _msg->deinterlace_mode.assign(decode_url(_options[i].substr(19)));
            else if( _options[i].length() && !_msg->url.compare("") )
                _msg->url.assign(decode_url(_options[i]));
        }
        
        if( _int_ad )
            _msg->play_type = P2P_PLAY_INTERRUPTABLE_AD;
        else if( _ad )
            _msg->play_type = P2P_PLAY_AD;
        else if( _stream )
            _msg->play_type = P2P_PLAY_STREAM;
        else
            _msg->play_type = P2P_PLAY_MAIN;
    }
    
    return _msg;
}

play_in_msg *In::play( const string &msg )
{
    play_in_msg *_msg = new play_in_msg;
    _msg->raw = msg;
    _msg->url = msg.substr(5);
    _msg->play_type = P2P_PLAY_MAIN;
    return _msg;
}

play_in_msg *In::play_ad( const string &msg )
{
    play_in_msg *_msg = new play_in_msg;
    _msg->raw = msg;
    _msg->url = msg.substr(7);
    _msg->play_type = P2P_PLAY_AD;
    return _msg;
}

play_in_msg *In::play_interruptable_ad( const string &msg )
{
    play_in_msg *_msg = new play_in_msg;
    _msg->raw = msg;
    _msg->url = msg.substr(8);
    _msg->play_type = P2P_PLAY_INTERRUPTABLE_AD;
    return _msg;
}

pause_in_msg *In::pause( const string &msg )
{
    pause_in_msg *_msg = new pause_in_msg;
    _msg->raw = msg;
    return _msg;
}

resume_in_msg *In::resume( const string &msg )
{
    resume_in_msg *_msg = new resume_in_msg;
    _msg->raw = msg;
    return _msg;
}

stop_in_msg *In::stop( const string &msg )
{
    stop_in_msg *_msg = new stop_in_msg;
    _msg->raw = msg;
    return _msg;
}

load_in_msg *In::load( const string &msg, bool sync )
{
    load_in_msg *_msg = new load_in_msg;
    _msg->raw = msg;
    _msg->load_id = 0;
    _msg->status = IN_LOAD_UNDF;
    _msg->err_message = "";
    _msg->infohash = "";
    _msg->checksum = "";
    _msg->default_quality = -1;

    string _base = sync ? msg : msg.substr(9);
    size_t _pos;
    if( !sync ) {
        _pos = _base.find("{");
        if( _pos != string::npos )
            _msg->load_id = atoi( _base.substr(0, _pos - 1).c_str() );
        _base = _base.substr( _pos+1, _base.length() - 1 );
    }
    
    _pos = _base.find( "\"status\":" );
    if( _pos != string::npos ) {
        size_t _pos_end = _base.find( ",", _pos );
        if( _pos_end != string::npos )
            _msg->status = (income_load_status_type)atoi(_base.substr( _pos + 10, _pos_end - _pos - 10).c_str());
        
        if( _msg->status == IN_LOAD_SINGLE || _msg->status == IN_LOAD_MULTI ) {
            _pos = _base.find("\"infohash\":");
            if( _pos != string::npos ) {
                 _pos_end = _base.find("\"", _pos + 13);
                 if( _pos_end != string::npos ) {
                    _msg->infohash = _base.substr( _pos + 13, _pos_end - _pos - 13 );
                    
                    _pos = _base.find("\"checksum\":");
                    if( _pos != string::npos ) {
                        _pos_end = _base.find("\"", _pos + 13);
                        if( _pos_end != string::npos )
                            _msg->checksum = _base.substr( _pos + 13, _pos_end - _pos - 13 );
                    }
                    
                    _pos = _base.find("\"default_quality\":");
                    if( _pos != string::npos ) {
                        _pos_end = _base.find( ",", _pos );
                        
                        if( _pos_end != string::npos ) {
                            _msg->default_quality = atoi(_base.substr( _pos + 19, _pos_end - _pos - 19).c_str());
                        }
                        else {
                            _pos_end = _base.find( "}", _pos );
                            if( _pos_end != string::npos )
                                _msg->default_quality = atoi(_base.substr( _pos + 19, _pos_end - _pos - 19).c_str());
                        }
                    }
                    
                    _pos = _base.find("\"qualities\":");
                    if( _pos != string::npos ) {
                        _pos_end = _base.find("}]", _pos + 15);
                        if( _pos_end != string::npos ) {
                            string q_str = _base.substr( _pos + 15, _pos_end - _pos - 15 );
                            do {
                                _pos = q_str.find("}, {");
                                string _item = ( _pos == string::npos ) ? q_str : q_str.substr( 0, _pos );
                                q_str.erase( 0, _pos+4 );
                                
                                size_t _pos_delim = _item.find(", ");
                                string _name = "";
                                string _bitrate = "";
                                if( _pos_delim != string::npos) {
                                    string _first = _item.substr( 0, _pos_delim );
                                    string _second = _item.substr( _pos_delim + 2, _item.length() - 1 );
                                    if( _first.find("\"bitrate\":") != string::npos )
                                        _bitrate = _first.substr( 11, _first.length() - 11 );
                                    else
                                        _name = _first.substr( 9, _first.length() - 10 );
                                        
                                    if( _second.find("\"name\": ") != string::npos )
                                        _name = _second.substr( 9, _second.length() - 10 );
                                    else
                                        _bitrate = _second.substr( 11, _second.length() - 11 );
                                }
                                
                                _msg->qualities.push_back( _name + " " + _bitrate + " Kbit/s" );
                            }
                            while( _pos != string::npos );
                        }
                    }
                    
                    _pos =  _base.find( "\"files\":" );
                    if( _pos != string::npos ) {
                        _pos_end = _base.find("]]");
                        if( _pos_end != string::npos )
                            _base = _base.substr( _pos + 11, _pos_end - _pos - 11 );

                        do {
                            _pos = _base.find("], [");
                            string _item = ( _pos == string::npos ) ? _base : _base.substr( 0, _pos );
                            _base.erase( 0, _pos + 4 );
                                                  
                            size_t _pos_delim = _item.find(", ");
                            if( _pos_delim != string::npos ) {
                                string _title, _index;

                                _title = _item.substr( 1, _pos_delim - 2 );
                                _title = In::decode_url(_title);
                                _index = _item.erase( 0, _pos_delim + 2 );
                                _msg->files.insert( pair<string, int>( _title, atoi(_index.c_str()) ) );
                            }
                        }
                        while( _pos != string::npos );
                    }
                    else {
                        _msg->status = IN_LOAD_ERROR;
                        _msg->err_message = "File without content.";
                    }
                 }
                 else {
                    _msg->status = IN_LOAD_ERROR;
                    _msg->err_message = "No infohash specified.";
                 }
            }
            else {
                _msg->status = IN_LOAD_ERROR;
                _msg->err_message = "No infohash specified.";
            }
        }
        else {
            if( _msg->status == IN_LOAD_ERROR ) {
                _pos = _base.find("\"message\":");
                if( _pos != string::npos ) {
                    _pos_end = _base.find("\"", _pos + 12);
                    if( _pos_end != string::npos )
                        _msg->err_message = _base.substr( _pos + 12, _pos_end - _pos - 12 );
                }
            }
            else {
                _msg->status = IN_LOAD_ERROR;
                _msg->err_message = "Cannot load transport file";
            }
        }
    }
    else {
        _msg->status = IN_LOAD_ERROR;
        _msg->err_message = "Cannot load transport file";
    }

    return _msg;
}

shutdown_in_msg *In::shutdown( const string &msg )
{
    shutdown_in_msg *_msg = new shutdown_in_msg;
    _msg->raw = msg;
    return _msg;
}

event_in_msg *In::event( const string &msg )
{
    event_in_msg *_msg = new event_in_msg;
    _msg->raw = msg;
    _msg->event_type = IN_EVENT_MSG_UNDF;
    _msg->event.cansave_event = NULL;
    _msg->event.dialog_event = NULL;
    _msg->event.live_pos_event = NULL;
    _msg->event.user_data_event = NULL;
    _msg->event.infowindow_event = NULL;
    
    string _base = msg.substr(6);
    vector<string> _options = In::split( _base, ' ' );
    
    if( !_options[0].compare( "cansave" ) ) {
        _msg->event_type = IN_EVENT_MSG_CANSAVE;
        _msg->event.cansave_event = new cansave_in_event_msg;
        _msg->event.cansave_event->infohash = "";
        _msg->event.cansave_event->index = -1;
        _msg->event.cansave_event->format = P2P_SAVE_UNSAVEABLE;
        
        for( size_t _i = 1; _i < _options.size(); ++_i ) {
            if( !_options[_i].compare(0, 8, "infohash") )
                _msg->event.cansave_event->infohash = _options[_i].substr(9);
            else if( !_options[_i].compare(0, 5, "index") )
                _msg->event.cansave_event->index = atoi(_options[_i].substr(6).c_str());
            else if( !_options[_i].compare(0, 6, "format") ) {
                string _format = _options[_i].substr(7);
                if( !_format.compare("plain") )
                    _msg->event.cansave_event->format = P2P_SAVE_PLAIN;
                else if( !_format.compare("encrypted") )
                    _msg->event.cansave_event->format = P2P_SAVE_ENCRYPTED;
            }
        }
    }
    else if( !_options[0].compare( "showdialog" ) ) {
        _msg->event_type = IN_EVENT_MSG_SHOW_DIALOG;
        _msg->event.dialog_event = new show_dialog_in_event_msg;
        _msg->event.dialog_event->text = "";
        _msg->event.dialog_event->title = "";
        _msg->event.dialog_event->provider_name = "";
        _msg->event.dialog_event->provider_url = "";
        _msg->event.dialog_event->premium_activate_url = "";
        
        for( size_t _i = 1; _i < _options.size(); ++_i ) {
            if( !_options[_i].compare(0, 4, "text") )
                _msg->event.dialog_event->text = decode_url(_options[_i].substr(5));
            else if( !_options[_i].compare(0, 5, "title") )
                _msg->event.dialog_event->title = decode_url(_options[_i].substr(6));
            else if( !_options[_i].compare(0, 13, "provider_name") )
                _msg->event.dialog_event->provider_name = decode_url(_options[_i].substr(14));
            else if( !_options[_i].compare(0, 12, "provider_url") )
                _msg->event.dialog_event->provider_url = decode_url(_options[_i].substr(13));
            else if( !_options[_i].compare(0, 20, "premium_activate_url") )
                _msg->event.dialog_event->premium_activate_url = decode_url(_options[_i].substr(21));
        }
    }
    else if( !_options[0].compare( "livepos" ) ) {
        _msg->event_type = IN_EVENT_MSG_LIVE_POS;
        _msg->event.live_pos_event = new live_pos_in_event_msg;
        _msg->event.live_pos_event->live_first = 0;
        _msg->event.live_pos_event->live_last = 0;
        _msg->event.live_pos_event->pos = 0;
        _msg->event.live_pos_event->first_ts = 0;
        _msg->event.live_pos_event->last_ts = 0;
        _msg->event.live_pos_event->last = 0;
        _msg->event.live_pos_event->buffer_pieces = 0;

        for( size_t _i = 1; _i < _options.size(); ++_i ) {
            if( !_options[_i].compare(0, 10, "live_first") )
                _msg->event.live_pos_event->live_first = atoi(_options[_i].substr(11).c_str());
            else if( !_options[_i].compare(0, 9, "live_last") )
                _msg->event.live_pos_event->live_last = atoi(_options[_i].substr(10).c_str());
            else if( !_options[_i].compare(0, 3, "pos") )
                _msg->event.live_pos_event->pos = atoi(_options[_i].substr(4).c_str());
            else if( !_options[_i].compare(0, 8, "first_ts") )
                _msg->event.live_pos_event->first_ts = atoi(_options[_i].substr(9).c_str());
            else if( !_options[_i].compare(0, 7, "last_ts") )
                _msg->event.live_pos_event->last_ts = atoi(_options[_i].substr(8).c_str());
            else if( !_options[_i].compare(0, 4, "last") )
                _msg->event.live_pos_event->last = atoi(_options[_i].substr(5).c_str());
            else if( !_options[_i].compare(0, 13, "buffer_pieces") )
                _msg->event.live_pos_event->buffer_pieces = atoi(_options[_i].substr(14).c_str());
            else if( !_options[_i].compare(0, 7, "is_live") )
                _msg->event.live_pos_event->is_live = (bool)atoi(_options[_i].substr(8).c_str());
        }
    }
    else if( !_options[0].compare( "getuserdata" ) ) {
        _msg->event_type = IN_EVENT_MSG_GET_USER_DATA;
        _msg->event.user_data_event = new user_data_in_event_msg;
    }
    else if( !_options[0].compare( "infowindow" ) ) {
        _msg->event_type = IN_EVENT_MSG_SHOW_INFOWINDOW;
        _msg->event.infowindow_event = new show_infowindow_in_event_msg;
        _msg->event.infowindow_event->type = "";
        _msg->event.infowindow_event->text = "";
        _msg->event.infowindow_event->height = 0;
        _msg->event.infowindow_event->buttons = 0;
        _msg->event.infowindow_event->btn1_action = 0;
        _msg->event.infowindow_event->btn1_text = "";
        _msg->event.infowindow_event->btn1_url = "";
        _msg->event.infowindow_event->btn2_action = 0;
        _msg->event.infowindow_event->btn2_text = "";
        _msg->event.infowindow_event->btn2_url = "";

        for( size_t _i = 1; _i < _options.size(); ++_i ) {
            if( !_options[_i].compare(0, 4, "type") ) {
                _msg->event.infowindow_event->type = decode_url(_options[_i].substr(5));
            }
            else if( !_options[_i].compare(0, 4, "text") ) {
                _msg->event.infowindow_event->text = decode_url(_options[_i].substr(5));
            }
            else if( !_options[_i].compare(0, 6, "height") ) {
                _msg->event.infowindow_event->height = atoi(_options[_i].substr(7).c_str());
            }
            else if( !_options[_i].compare(0, 7, "buttons") ) {
                _msg->event.infowindow_event->buttons = atoi(_options[_i].substr(8).c_str());
            }
            else if( !_options[_i].compare(0, 11, "btn1_action") ) {
                _msg->event.infowindow_event->btn1_action = atoi(_options[_i].substr(12).c_str());
            }
            else if( !_options[_i].compare(0, 9, "btn1_text") ) {
                _msg->event.infowindow_event->btn1_text = decode_url(_options[_i].substr(10));
            }
            else if( !_options[_i].compare(0, 8, "btn1_url") ) {
                _msg->event.infowindow_event->btn1_url = decode_url(_options[_i].substr(9));
            }
            else if( !_options[_i].compare(0, 11, "btn2_action") ) {
                _msg->event.infowindow_event->btn2_action = atoi(_options[_i].substr(12).c_str());
            }
            else if( !_options[_i].compare(0, 9, "btn2_text") ) {
                _msg->event.infowindow_event->btn2_text = decode_url(_options[_i].substr(10));
            }
            else if( !_options[_i].compare(0, 8, "btn2_url") ) {
                _msg->event.infowindow_event->btn2_url = decode_url(_options[_i].substr(9));
            }
        }
    }
    
    return _msg;
}

#define CHECK_GET_INT(a,b,c) \
{ \
    Json::Value v = value.get(b, c); \
    if(v.isInt()) { \
        a = v.asInt(); \
    } \
    else if(v.isString()) { \
        std::string str = v.asString(); \
        a = atoi(str.c_str()); \
    } \
}

#define CHECK_GET_BOOL(a,b,c) \
{ \
    Json::Value v = value.get(b, c); \
    if(v.isBool()) { \
        a = v.asBool(); \
    } \
    else if(v.isInt()) { \
        a = (bool)v.asInt(); \
    } \
    else if(v.isString()) { \
        std::string str = v.asString(); \
        if(!str.compare("true") || !str.compare("false")) { \
            a = !str.compare("true"); \
        } \
        else { \
            a = (bool)atoi(str.c_str()); \
        } \
    } \
}
static load_url_item parse_load_url_item(Json::Value value, int group) {
    load_url_item load_item;
    load_item.type = P2P_LOAD_URL_UNDF;

    string _type = value.get("type", "").asString();
    if( !_type.compare("overlay") )
        load_item.type = P2P_LOAD_URL_OVERLAY;
    else if( !_type.compare("interactive-pause") )
        load_item.type = P2P_LOAD_URL_PAUSE;
    else if( !_type.compare("interactive-stop") )
        load_item.type = P2P_LOAD_URL_STOP;
    else if( !_type.compare("interactive-preroll") )
        load_item.type = P2P_LOAD_URL_PREROLL;
    else if( !_type.compare("slider") )
        load_item.type = P2P_LOAD_URL_SLIDER;
    else if( !_type.compare("interactive-hidden") )
        load_item.type = P2P_LOAD_URL_HIDDEN;
    else if( !_type.compare("interactive-preplay") )
        load_item.type = P2P_LOAD_URL_PREPLAY;
    else if( !_type.compare("webstat-play") )
        load_item.type = P2P_LOAD_URL_WEBSTAT_PLAY;
    else if( !_type.compare("webstat-pause") )
        load_item.type = P2P_LOAD_URL_WEBSTAT_PAUSE;
    else if( !_type.compare("webstat-stop") )
        load_item.type = P2P_LOAD_URL_WEBSTAT_STOP;
    else if( !_type.compare("webstat-fullscreen") )
        load_item.type = P2P_LOAD_URL_WEBSTAT_FULLSCREEN;

    Json::Value urlList = value.get("urlList", Json::Value::null);
    if(urlList.isNull()) {
        load_item.ids.push_back(value.get("id", "").asString());
        load_item.urls.push_back(value.get("url", "").asString());
    }
    else {
        if(urlList.isArray()) {
            for(int j = 0; j < urlList.size(); ++j) {
                Json::Value urlListItem = urlList.get(j, Json::Value::null);
                if(!urlListItem.isNull() && urlListItem.isObject()) {
                    string id = urlListItem.get("id", "").asString();
                    string url = urlListItem.get("url", "").asString();
                    if(url.compare("")) {
                        load_item.ids.push_back(id);
                        load_item.urls.push_back(url);
                    }
                }
            }
        }
    }

    CHECK_GET_BOOL(load_item.require_flash, "requireFlash", false);
    CHECK_GET_INT(load_item.width, "width", 0);
    CHECK_GET_INT(load_item.height, "height", 0);
    CHECK_GET_INT(load_item.left, "left", -1);
    CHECK_GET_INT(load_item.top, "top", -1);
    CHECK_GET_INT(load_item.right, "right", -1);
    CHECK_GET_INT(load_item.bottom, "bottom", -1);
    CHECK_GET_INT(load_item.min_width, "minWidth", 0);
    CHECK_GET_INT(load_item.min_height, "minHeight", 0);
    CHECK_GET_BOOL(load_item.allow_dialogs, "allowDialogs", false);
    CHECK_GET_BOOL(load_item.allow_window_open, "allowWindowOpen", true);
    CHECK_GET_BOOL(load_item.enable_flash, "enableFlash", true);
    CHECK_GET_INT(load_item.cookies, "cookies", 1);

    load_item.embed_code = value.get("embedCode", "").asString();

    CHECK_GET_BOOL(load_item.preload, "preload", true);
    CHECK_GET_INT(load_item.fullscreen, "fullscreen", 0);

    int maxi;
    CHECK_GET_INT(maxi, "maxImpressions", 0);
    load_item.max_impressions = maxi == 0 ? 9999 : maxi;
    load_item.impressions = 0;

    load_item.content_type = value.get("contentType", "").asString();
    load_item.creative_type = value.get("creativeType", "").asString();
    load_item.click_url = value.get("clickUrl", "").asString();

    CHECK_GET_INT(load_item.user_agent, "userAgent", 1);
    CHECK_GET_INT(load_item.close_after_seconds, "closeAfter", 0);
    CHECK_GET_INT(load_item.show_time, "showTime", 0);
    CHECK_GET_BOOL(load_item.start_hidden, "startHidden", false);
    CHECK_GET_BOOL(load_item.url_filter, "enableUrlFilter", false);
    CHECK_GET_BOOL(load_item.useIE, "useIE", false);

    load_item.group_id = group;

    Json::Value eScripts = value.get("embedScripts", Json::Value::null);
    if(!eScripts.isNull()) {
        for(int j = 0; j < eScripts.size(); ++j) {
            string val = eScripts.get(j, "").asString();
            if(val.compare("")) {
                load_item.embed_scripts.push_back(val);
            }
        }
    }

    return load_item;
}
#undef CHECK_GET_BOOL
#undef CHECK_GET_INT

static void parse_load_url_array(Json::Value value, int group_id, load_url_msg *_msg) {
    for(int i = 0; i < value.size(); ++i) {
        Json::Value item = value.get(i, Json::Value::null);
        if(!item.isNull()) {
            if(item.isArray()) {
                int _group_id = rand() % 9999 + 1000;
                parse_load_url_array(item, _group_id, _msg);
            }
            else if(item.isObject()) {
                load_url_item load_item = parse_load_url_item(item, group_id);
                _msg->items.push_back(load_item);
            }
        }
    }
}

load_url_msg *In::load_url( const string &msg )
{
    load_url_msg *_msg = new load_url_msg;
    _msg->raw = msg;
    
    string _base = decode_url(msg);
    if(!_base.compare(0, 15, "LOAD_URL items=")) {
        _base.erase(0, 15);
    }

    Json::Value root;
    Json::Reader reader;
    bool parsed = reader.parse(_base, root);
    if(!parsed) {
        _msg->error = reader.getFormatedErrorMessages();
        return _msg;
    }
    if(!root.isArray()) {
        _msg->error = "Not array value";
        return _msg;
    }
    parse_load_url_array(root, 0, _msg);
    
    return _msg;
}