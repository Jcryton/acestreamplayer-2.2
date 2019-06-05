#ifndef __P2P_PLUGIN_DB__HEADER__
#define __P2P_PLUGIN_DB__HEADER__

#include <vlc_sql.h>
#include <p2p_object.h>
#include <string>
#include <map>

#define DB_VERSION "2.0"
#define DN_NAME "options.sdb"

class DB
{
    enum result_type {
        DB_VERSION_TYPE,
        DB_OPTIONS_TYPE,
    };
    struct db_result {
        result_type type;
        virtual ~db_result() {};
    };
    struct db_version_result : db_result {
        std::string version;
        db_version_result() : db_result() { type = DB_VERSION_TYPE; }
    };
    struct db_options_result : db_result {
        std::map<std::string, std::string> options;
        db_options_result() : db_result() { type = DB_OPTIONS_TYPE; }
    };
    
public:
    DB( p2p_object_t* );
    ~DB();
    
    bool connect();
    bool save( std::string, std::string, std::string );
    std::map<std::string, std::string> retrieve( std::string infohash );

private:
    bool createEmptyDatabase( bool = true );
    bool rebuildOldDatabase( std::string );
    std::string getDBVersion();
    
    std::string prepare( const char *format, ... );
    bool exec( std::string, db_result* = NULL );
    
    void toDBResult( char**, int, int, db_result* );
private:
    p2p_object_t* m_vlcobj;
    sql_t *m_sql;
};

#endif