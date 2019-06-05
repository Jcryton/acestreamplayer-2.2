#ifndef __P2P_PLUGIN_CONTROL__HEADER__
#define __P2P_PLUGIN_CONTROL__HEADER__

#include <p2p_object.h>
#include <string>
#include <vector>
#include <sstream>

#include "api/api.h"
#include "p2p_access.h"

#ifdef WIN32
#ifdef TORRENT_STREAM
const wchar_t REG_SECTION[] = L"Software\\TorrentStream";
#else
const wchar_t REG_SECTION[] = L"Software\\ACEStream";
#endif
const wchar_t REG_ENGINE_KEY[] = L"EnginePath";
const wchar_t REG_IE_SECTION[] = L"SOFTWARE\\Microsoft\\Internet Explorer";
const wchar_t REG_IEVERSION_KEY[] = L"Version";
const wchar_t REG_INSTALL_KEY[] = L"InstallDir";
const wchar_t PORT_FILE[] = L"\\engine\\acestream.port";
#else
const char UNIX_PATH[] = "/usr/bin/acestreamengine";
#endif

class Connection;
class DB;
class Control 
{
public:
    Control( p2p_object_t * );
    ~Control();
    
    bool isReady();
    bool isShutdown();
    
    bool startup();
    void shutdown();
    void ready();
    
    bool send( base_out_message* );
    bool sendString( std::string& );
    base_in_message *sendSync( base_out_message* );
    bool receive( std::string& );
    void processEngineMessage( base_in_message* );
    int processEngineLoadMessage( load_in_msg*, async_loading_item );
    
    bool dbSaveOption( std::string, std::string, std::string );
    bool supportedStatEvents() { return m_version_options.support_stat_events; }
    bool supportedTenAgesUserinfo() { return m_version_options.support_ten_ages_userinfo; }
    
    void requestLoadUrl(int, int = 0);
    void registerLoadUrlStatistics(int, int, std::string);
    void registerLoadUrlEvent(int, std::string, std::string);
    void clearLoadUrl();
    
private:
#ifdef WIN32
    bool readPortFromFile();
    int readFile( std::string );
    std::string getIEVersion();
#endif
    bool processConnect2Engine();
    bool helloEngine();
    bool startEngine();
    
    void versionProcess( int, int, int, int, bool = false );
    
    void prepareLoadUrlItems(load_url_msg *);
    
private:
    p2p_object_t *m_vlcobj;
    Connection *m_connection;
    DB *m_db;
    bool m_closing;
    bool m_ready;
    bool m_shutdown;
    bool m_db_enabled;
    std::string m_ready_key;
    bool m_remote_engine;
    bool m_preplay_requested;
    
    std::vector<load_url_item> m_slider_items;
    std::vector<load_url_item> m_overlay_items;
    std::vector<load_url_item> m_pause_items;
    std::vector<load_url_item> m_stop_items;
    std::vector<load_url_item> m_preroll_items;
    std::vector<load_url_item> m_hidden_items;
    std::vector<load_url_item> m_preplay_items;
    std::vector<load_url_item> m_webstat_items;
    
    struct version_options {
        bool support_stat_events;
        bool support_ten_ages_userinfo;
        bool wait_preplay_loadurls;
    } m_version_options;
};

#endif