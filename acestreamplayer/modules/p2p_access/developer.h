#ifndef __P2P_PLUGIN_DEVELOPER__HEADER__
#define __P2P_PLUGIN_DEVELOPER__HEADER__

#include <gcrypt.h>

/*static const short developer_key[] = {

};

char *developer_to_string( const short *arr, size_t *len ) 
{
    char *str;
    if( *len == 1 && arr[0] == 0) {
        str = (char*)malloc(1);
        str[0] = 0;
        *len = 0;
        return str;
    }
    
    str = (char*)malloc( *len+1 );
    for( size_t i = 0; i < *len; ++i ) {
        str[i] = (char)(arr[i] ^ (((i<<8) * 14 + 17 -i) % 256));
    }
    str[*len] = 0;
    return str;
}*/

char *sha1( const char *psz_string ) 
{
   size_t len = strlen( psz_string );
   size_t hash_len = gcry_md_get_algo_dlen( GCRY_MD_SHA1 );
   unsigned char hash[ hash_len ];

   char *out = (char *) malloc( sizeof(char) * ((hash_len*2)+1) );
   char *p = out;
   gcry_md_hash_buffer( GCRY_MD_SHA1, hash, psz_string, len );

   for( size_t i = 0; i < hash_len; i++, p += 2 ) {
      snprintf( p, 3, "%02x", hash[i] );
   }
   return out;
}

#endif