#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_network.h>
#include <vlc_fs.h>
#include <sys/stat.h>

#include "db.h"

using namespace std;

DB::DB( p2p_object_t *vlc_obj ) : m_vlcobj(vlc_obj), m_sql(NULL)
{
    msg_P2PLog( vlc_obj, "[DB]: Initializing database" );
}

DB::~DB()
{
    msg_P2PLog( m_vlcobj, "[DB]: Deinitializing database" );
    if( m_sql )
        sql_Destroy( m_sql );
}

bool DB::connect()
{
    char *_psz_app_datatdir = config_GetUserDir( VLC_DATA_DIR );

    struct stat st;
    if( vlc_stat( _psz_app_datatdir, &st ) != 0 ) {
        if( vlc_mkdir( _psz_app_datatdir, 0700 ) != 0 ) {
            free( _psz_app_datatdir );
            msg_P2PLog( m_vlcobj, "[DB::connect]: cannot create %s", _psz_app_datatdir );
            return false;
        }
    }
    
    string host( _psz_app_datatdir );
    free( _psz_app_datatdir );
    
    host.append(DIR_SEP).append(DN_NAME);
    m_sql = sql_Create( m_vlcobj, NULL, host.c_str(), 0, "", "" );
    if( !m_sql ) {
        msg_P2PLog( m_vlcobj, "[DB::connect]: Error creating sql_t" );
        m_sql = NULL;
        return false;
    }
    
    string _version = getDBVersion();
    msg_P2PLog( m_vlcobj, "[DB::connect]: got db version %s", _version.c_str() );
    if( _version == "" )
        return createEmptyDatabase();
    else if( _version != DB_VERSION )
        return rebuildOldDatabase(_version);
    else
        return true;
}

bool DB::save( string infohash, string name, string value )
{
    if( !m_sql )
        return false;
    
    bool _res = false;
    if( value == "" ) {
        string _query = prepare("DELETE FROM options WHERE infohash = '%s' and name = '%s'", infohash.c_str(), name.c_str());
        _res = exec(_query);
        return _res;
    }

    db_options_result _result;
    string _query = prepare("SELECT name, value FROM options WHERE infohash = '%s' and name = '%s' LIMIT 1", infohash.c_str(), name.c_str());
    _res = exec(_query, &_result);
    if( _res ) {
        if( _result.options.size() > 0 ) {
            _query = prepare("UPDATE options SET value = '%s' WHERE infohash = '%s' AND name = '%s'", value.c_str(), infohash.c_str(), name.c_str());
            _res = exec(_query);
        }
        else {
            _query = prepare("INSERT INTO options (infohash, name, value) VALUES ('%s', '%s', '%s')", infohash.c_str(), name.c_str(), value.c_str());
            _res = exec(_query);
        }
    }
    return _res;
}

map<string, string> DB::retrieve( string infohash )
{
    map<string, string> opt_dict;
    db_options_result _result;
    string _query = prepare("SELECT name, value FROM options WHERE infohash = '%s'", infohash.c_str());
    bool _res = exec(_query, &_result);
    if( _res )
        opt_dict = _result.options;
    return opt_dict;
}

bool DB::createEmptyDatabase( bool with_transaction )
{
    bool _ret = true;
    msg_P2PLog( m_vlcobj, "[DB]: Creating a new (empty) database.." );
    
    if( with_transaction )
        sql_BeginTransaction( m_sql );
    
    _ret = exec("CREATE TABLE options( infohash VARCHAR(50), name VARCHAR(128) NOT NULL, value VARCHAR(128) NOT NULL, PRIMARY KEY(infohash, name))");
    if( _ret ) {
        _ret = exec("CREATE TABLE info(name VARCHAR(128) PRIMARY KEY, value VARCHAR(128))");
        if( _ret ) {
             string _query = prepare("INSERT INTO info(name, value) VALUES('version', '%s')", DB_VERSION);
            if( _query != "" ) {
                _ret = exec(_query);
            }
            else
                _ret = false;
        }
    }
    if( with_transaction )
        if( _ret )
            sql_CommitTransaction( m_sql );
        else
            sql_RollbackTransaction( m_sql );
    
    return _ret;
}

bool DB::rebuildOldDatabase( string version )
{
    (void)version;
    bool _ret = true;
    sql_BeginTransaction( m_sql );
    
    _ret = exec("DROP TABLE IF EXISTS options");
    if( _ret ) {
        _ret = exec("DROP TABLE IF EXISTS info");
        if( _ret )
            _ret = createEmptyDatabase(false);
    }
    
    if( _ret )
        sql_CommitTransaction( m_sql );
    else
        sql_RollbackTransaction( m_sql );
    return _ret;
}

string DB::getDBVersion()
{
    int _rows, _cols;
    char **pp_results;

    db_version_result _result;
    bool _res = exec("SELECT value FROM info WHERE name = 'version' LIMIT 1", &_result);
    return _res ? _result.version : "";
}

string DB::prepare( const char *format, ... )
{
    va_list args;
    va_start( args, format );
    
    string _ret = "";
    char *_query = sql_VPrintf( m_sql, format, args );
    if( _query ) {
        _ret.assign(_query);
        free(_query);
    }
    
    va_end( args );
    return _ret;
}

bool DB::exec( string query, db_result *results )
{
    int _rows, _cols;
    char **pp_results = NULL;
    
    int _res = sql_Query( m_sql, query.c_str(), &pp_results, &_rows, &_cols );
    if( results )
        toDBResult( pp_results, _rows, _cols, results );
    if( pp_results )
        sql_Free( m_sql, pp_results );

    return _res == VLC_SUCCESS;
}

void DB::toDBResult( char **pp_results, int _rows, int _cols, db_result *results )
{
    if( !results )
        return;
    
    switch( results->type ) {
        case DB_VERSION_TYPE : {
            if( pp_results ) {
                db_version_result *ver = static_cast<db_version_result *>(results);
                if( _cols != 1 && _rows != 1 ) {
                    ver->version = "";
                    break;
                }
                ver->version = pp_results[1] ? string( pp_results[1]  ) : "";
            }
            break;
        }
        case DB_OPTIONS_TYPE : {
            if( pp_results ) {
                db_options_result *opt = static_cast<db_options_result *>(results);
                map<string, string> opt_dict;
                if( _cols != 2 && _rows <= 0 ) {
                    opt->options = opt_dict;
                    break;
                }
                for( size_t i = 1; i <= _rows; ++i ) {
                    if( pp_results[i * 2] && pp_results[i * 2 + 1] ) {
                        opt_dict.insert( pair<string,string>(string( pp_results[i * 2] ), string( pp_results[i * 2 + 1] )) );
                    }
                }
                opt->options = opt_dict;
            }
            break;
        }
        default:
            break;
    }
}