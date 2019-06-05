#ifndef __P2P_PLUGIN_OUTCOME__HEADER__
#define __P2P_PLUGIN_OUTCOME__HEADER__

#include "api.h"

class Out
{
public:
    static std::string Build( base_out_message *msg );

private:
    static std::string hello( hello_out_msg* );
    static std::string ready( ready_out_msg* );
    static std::string load( load_out_msg* );
    static std::string load_async( load_async_out_msg* );
    static std::string start( start_out_msg* );
    static std::string stop( stop_out_msg* );
    static std::string duration( duration_out_msg* );
    static std::string playback( playback_out_msg* );
    static std::string save( save_out_msg* );
    static std::string get_pid( get_pid_out_msg* );
    static std::string get_cid( get_cid_out_msg* );
    static std::string get_ad_url( get_ad_url_out_msg* );
    static std::string live_seek( live_seek_out_msg* );
    static std::string user_data( user_data_out_msg* );
    static std::string shutdown( shutdown_out_msg* );
    static std::string stat_event( stat_event_out_msg* );
    static std::string load_url_event( load_url_event_out_msg* );
    static std::string user_data_mining( user_data_mining_out_msg* );
    static std::string infowindow_response( infowindow_response_out_msg* );
};

#endif