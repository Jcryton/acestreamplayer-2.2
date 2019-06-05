#ifndef __P2P_PLUGIN__HEADER__
#define __P2P_PLUGIN__HEADER__

#include <vlc_common.h>
#include <p2p_object.h>
#include <vlc_playlist.h>
#include <vlc_dialog.h>
#include <vlc_fs.h>
#include <vlc_url.h>
#include <vlc_charset.h>
#include <map>
#include <vector>

struct async_loading_item
{
    std::string id;
    std::string name;
    p2p_uri_id_type_t type;
    std::vector<std::string> options;
    bool playlist;
    int add_mode;
    int add_type;
    playlist_item_t *parent;
    int developer;
    int affiliate;
    int zone;
    bool locked;
};

class Control;
struct p2p_object_sys_t
{
    vlc_thread_t p_thread;
    
    Control *p_control;
    bool will_play;
    
    p2p_state_t state;
    int auth;
    
    std::map<int, async_loading_item> *async_loading_items;
    std::vector<std::string> *out_messages_queue;
    std::vector<int> *groups_register;
    std::string last_start_cmd_string;
    std::map<p2p_command_callback_type, std::pair<p2p_common_callback, void*> > *callbacks;
};

int LoadTorrentWithOptArray( p2p_object_t*, const char*, const char*, p2p_uri_id_type_t, bool, int, const char* const*, int, int, playlist_item_t*, bool, bool, int, int, int, bool );
int LoadTorrentWithOptString( p2p_object_t*, const char*, const char*, p2p_uri_id_type_t, bool, const char*, int, int, playlist_item_t*, bool, bool, int, int, int, bool );

bool Start( p2p_object_t*, const char*, const char*, p2p_uri_id_type_t, int, int, int, int, int );
bool Stop( p2p_object_t* );

bool Duration( p2p_object_t*, const char*, int );
bool Playback( p2p_object_t*, const char*, p2p_playback_value_t );

char* GetPid( p2p_object_t*, const char*, int, int, int );
char* GetCid( p2p_object_t*, const char*, const char*, int, int, int );

bool Save( p2p_object_t*, const char*, int, const char* );
bool GetAdUrl(p2p_object_t*, const char*, const char*, int, int);
bool LiveSeek(p2p_object_t*, int);
bool UserData( p2p_object_t*, int, int );
bool UserDataMining( p2p_object_t*, int );
bool InfoWindowResponse( p2p_object_t*, const char*, int );

bool StatisticsEvent(p2p_object_t*, p2p_statistics_event_type_t, int);

bool SaveOption(p2p_object_t*, const char*, const char*, const char*);

void SetCallback(p2p_object_t*, p2p_command_callback_type, p2p_common_callback, void*);

void RequestLoadUrlAd(p2p_object_t*, p2p_load_url_type_t, int);
void RegistedLoadUrlAdStatistics(p2p_object_t*, p2p_load_url_type_t, p2p_load_url_statistics_event_type_t, const char*);
void RegistedLoadUrlAdEvent(p2p_object_t*, p2p_load_url_type_t, const char*, const char*);

int generate_new_async_id( p2p_object_t* );
int generate_new_group_id( p2p_object_t* );
bool check_load_response( p2p_object_t *, load_in_msg *, std::string );
void set_message_for_error_dialog( p2p_object_t *, std::string, const char *, ... );

void RestartLast( p2p_object_t* );

#endif