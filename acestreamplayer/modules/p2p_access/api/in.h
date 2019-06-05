#ifndef __P2P_PLUGIN_INCOME__HEADER__
#define __P2P_PLUGIN_INCOME__HEADER__

#include "api.h"

class In
{
public:
    static base_in_message *Parse( const std::string& );
    static base_in_message *ParseSyncLoad( const std::string& );
    static base_in_message *ParseSyncGetPid( const std::string& );
    static base_in_message *ParseSyncGetCid( const std::string& );
    
public:
    static std::vector<std::string> split( const std::string &, char );
    static std::string decode_url( std::string );
    static bool is_numeric( std::string & );

private:
    static notready_in_msg *notready( const std::string& );
    static hello_in_msg *hello( const std::string& );
    static auth_in_msg *auth( const std::string& );
    static status_in_msg *status( const std::string& );
    static state_in_msg *state( const std::string& );
    static info_in_msg *info( const std::string& );
    static play_in_msg *start( const std::string& );
    static play_in_msg *play( const std::string& );
    static play_in_msg *play_ad( const std::string& );
    static play_in_msg *play_interruptable_ad( const std::string& );
    static pause_in_msg *pause( const std::string& );
    static resume_in_msg *resume( const std::string& );
    static stop_in_msg *stop( const std::string& );
    static load_in_msg *load( const std::string&, bool = false );
    static shutdown_in_msg *shutdown( const std::string& );
    static event_in_msg *event( const std::string& );
    static load_url_msg *load_url( const std::string& );
};

#endif