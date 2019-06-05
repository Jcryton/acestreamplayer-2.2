#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vlc_common.h>
#include <p2p_object.h>
#include <vlc_playlist.h>
#include "p2p_object_internal.h"

#if defined( WIN32 )
#include <shellapi.h>
#else
#include <stdlib.h>
#endif

static void TimerTick( void * );
static void OpenBrowser( const char* );

p2p_video_click_t *p2p_InitVideoClick( p2p_object_t *p_p2p )
{
    p2p_video_click_t *p_vc = (p2p_video_click_t*)malloc( sizeof(p2p_video_click_t) );
    if( !p_vc )
        return NULL;
    p_vc->b_timer_enabled = false;
    p_vc->psz_clickurl = NULL;
    
    if( vlc_timer_create( &p_vc->timer, TimerTick, p_p2p ) ) {
        msg_P2PLog(p_p2p, "[p2p_video_click.c] cannot create p2p timer");
        free( p_vc );
        return NULL;
    }
    else
        p_vc->b_timer_enabled = true;
    
    return p_vc;
}

void p2p_UnInitVideoClick( p2p_video_click_t *p_vc )
{
    if( p_vc ) {
        vlc_timer_destroy( p_vc->timer );
        if( p_vc->psz_clickurl )
            free( p_vc->psz_clickurl );
        free( p_vc );
    }
}

void p2p_SetVideoClickState( p2p_video_click_t *p_vc, bool active, const char *clickurl )
{
    if( p_vc->b_timer_enabled ) {
        if( active ) {
            if( strcmp(clickurl, "") ) {
                if( p_vc->psz_clickurl )
                    free( p_vc->psz_clickurl );
                p_vc->psz_clickurl = strdup( clickurl );
                    
                vlc_timer_schedule( p_vc->timer, false, 1000000, 0 );
            }
        }
        else {
            vlc_timer_schedule( p_vc->timer, false, 0, 0 );
            
            if( p_vc->psz_clickurl ) {
                free( p_vc->psz_clickurl );
                p_vc->psz_clickurl = NULL;
            }
        }
    }
}

static void TimerTick( void *data )
{
    p2p_object_t *p_p2p = (p2p_object_t*)data;
    
    vlc_mutex_lock( &p_p2p->lock );
    if( p_p2p->p_vclick->psz_clickurl ) {
        msg_P2PLog(p_p2p, "[p2p_video_click.c] trying to open: %s", p_p2p->p_vclick->psz_clickurl);
        OpenBrowser( p_p2p->p_vclick->psz_clickurl );
        free( p_p2p->p_vclick->psz_clickurl );
        p_p2p->p_vclick->psz_clickurl = NULL;
        
        var_TriggerCallback( p_p2p, "exit-fullscreen" );
    }
    vlc_mutex_unlock( &p_p2p->lock );
}

static void OpenBrowser( const char *url )
{
#if defined(WIN32)
    ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    char command[512];
    strcpy( command, "open " );
    strcat( command, url );
    system( command );
#elif defined(__linux__)
    char command[512];
    strcpy( command, "xdg-open " );
    strcat( command, url );
    system( command );
#else
#warning "OpenBrowser not realised for your platform"
#endif
}
