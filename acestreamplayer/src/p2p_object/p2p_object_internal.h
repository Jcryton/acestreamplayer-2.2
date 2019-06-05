#ifndef __LIBVLC_P2POBJECT_INTERNAL_H
#define __LIBVLC_P2POBJECT_INTERNAL_H 1

p2p_object_t *p2p_Create( vlc_object_t* );
void p2p_Destroy( p2p_object_t* );

static char *strncpy_from (char * dest, const char * source, size_t from, size_t count)
{
    char *start = dest;
    while (from--&& *source++)
        ;
    while (count && (*dest++ = *source++))
        count--;
    if (count)
        while (--count)
            *dest++ = '\0';
    return(start);
}

static char *strsub( const char *str, const char *match )
{
    return strstr( str, match );
}

struct p2p_video_click_t {
    vlc_timer_t timer;
    bool b_timer_enabled;
    char *psz_clickurl;
};

p2p_video_click_t *p2p_InitVideoClick( p2p_object_t* );
void p2p_UnInitVideoClick( p2p_video_click_t* );
void p2p_SetVideoClickState( p2p_video_click_t*, bool, const char* );

#endif