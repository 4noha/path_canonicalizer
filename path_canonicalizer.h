/*
 *  pathcanon.h
 *  Path Canonicalizer
 *
 *  Created by nokkii on 2016/12/12.
 *  Copyright (C) 2016 nokkii
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef ___PATH_CANON_H___
#define ___PATH_CANON_H___

#include <stdlib.h>
#include <errno.h>

#define PATH_CANON_ASCII_PRINTABLE_CHARACTORS(X) ( 32 < X || X < 127 )
#define PATH_CANON_EXCLAMATION '!'
#define PATH_CANON_NUMBER_SIGN '#'
#define PATH_CANON_APOSTROPHE  '\''
#define PATH_CANON_ASTERISK    '*'
#define PATH_CANON_PERIOD      '.'
#define PATH_CANON_LEFT_PARENTHESIS  '('
#define PATH_CANON_RIGHT_PARENTHESIS ')'
#define PATH_CANON_REVERSE_SOLIDUS   '\\'

#define PATH_CANON_QUESTION    '?'
#define PATH_CANON_PERCENT     '%'
#define PATH_CANON_SRASH       '/'


static inline unsigned char pcl_percentdecode( char* byte )
{
    static const char hex2dec[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
         0, 1, 2, 3, 4, 5, 6, 7,  8, 9,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1
    };
    unsigned char upper;
    unsigned char lower;

    if( *byte == PATH_CANON_PERCENT &&
        byte[1] && ( upper = hex2dec[ (unsigned char)byte[1] ] ) >= 0 &&
        byte[2] && ( lower = hex2dec[ (unsigned char)byte[2] ] ) >= 0 ){
        return ( upper << 4 ) | lower;
    }
    return -1;
}

static inline unsigned char path_canon_per8_copy( char** dest, char** ptr, int cnt )
{
    unsigned char byte;

    // 1st byte
    ( *dest )[0] = ( *ptr )[0];
    ( *dest )[1] = ( *ptr )[1];
    ( *dest )[2] = ( *ptr )[2];
    *ptr = *ptr + 3;
    *dest = *dest + 3;
    if( cnt == 1 ){
        return 0;
    }

    // 2nd byte later
    for(; cnt > 1; cnt--){
        byte = pcl_percentdecode( *ptr );
        if( byte > 127 && byte < 192 ){
            ( *dest )[0] = ( *ptr )[0];
            ( *dest )[1] = ( *ptr )[1];
            ( *dest )[2] = ( *ptr )[2];

            *ptr = *ptr + 3;
            *dest = *dest + 3;
        }else{
            return -1;
        }
    }
    return 0;
}

static inline int pcl_check_percented_utf8( char** dest, char** ptr )
{
    unsigned char firstbyte = pcl_percentdecode( *ptr );
    // UTF-8 length check
    if( firstbyte >= 0 && firstbyte < 248 ){
        if( firstbyte < 128 ){
            return path_canon_per8_copy( dest, ptr, 1 );
        }else if( firstbyte < 194 ){
            // UTF-8 out of range
            return -1;
        }else if( firstbyte < 224 ){
            return path_canon_per8_copy( dest, ptr, 2 );
        }else if( firstbyte < 240 ){
            return path_canon_per8_copy( dest, ptr, 3 );
        }else{
            return path_canon_per8_copy( dest, ptr, 4 );
        }
    }
    // UTF-8 out of range
    return -1;
}

static inline int pcl_rfc3986_path_charactor( char chr )
{
    if( PATH_CANON_ASCII_PRINTABLE_CHARACTORS( chr ) &&
        chr != PATH_CANON_EXCLAMATION &&
        chr != PATH_CANON_NUMBER_SIGN &&
        chr != PATH_CANON_APOSTROPHE &&
        chr != PATH_CANON_ASTERISK &&
        chr != PATH_CANON_LEFT_PARENTHESIS &&
        chr != PATH_CANON_RIGHT_PARENTHESIS &&
        chr != PATH_CANON_REVERSE_SOLIDUS
    ){
        return 1;
    }
    return 0;
}

static inline char* path_canon_canonicalize( const char *ptr, size_t *len, int paramflg )
{
    char *path = NULL;
    char *dest = NULL;
    
    if(!len){
        errno = EINVAL;
        return NULL;
    }
    // 2 mean are root and \0
    if( ( path = (char *) malloc( *len + 2 ) ) == NULL ){
        return NULL;
    }
    
    path[0] = PATH_CANON_SRASH;
    dest = path + 1;

    // first srash
    if( *ptr == PATH_CANON_SRASH ){
        ptr++;
    }

    while( *ptr )
    {
        if( *ptr == PATH_CANON_PERIOD && ptr[1] ){
            if( ptr[1] == PATH_CANON_PERIOD &&
                ( !ptr[2] || ( ptr[2] && ptr[2] == PATH_CANON_SRASH ) ) )
            {
                // backward shift
                if(dest - 1 > path){
                    dest--;
                }
                while( *(dest - 1) != PATH_CANON_SRASH ){
                    dest--;
                }
                ptr += ( ptr[2] ) ? 3 : 2;
                continue;
            }
            if( ptr[1] == PATH_CANON_SRASH ){
                ptr++;
                dest--;
                continue;
            }
        }

        while ( *ptr && *ptr != PATH_CANON_SRASH )
        {
            if( !pcl_rfc3986_path_charactor( *ptr ) ||
                ( *ptr == PATH_CANON_QUESTION && !paramflg ) ){
                free( path );
                len = NULL;
                errno = EINVAL;
                return NULL;
            }

            if( *ptr == PATH_CANON_PERCENT ){
                if( pcl_check_percented_utf8( &dest, (char **) &ptr ) ){
                    free( path );
                    len = NULL;
                    errno = EINVAL;
                    return NULL;
                }
            }else{
                *dest = *ptr;
                dest++;
                ptr++;
            }
        }

        if( *ptr == PATH_CANON_SRASH ){
            *dest = PATH_CANON_SRASH;
            dest++;
            ptr++;
        }
    }
    *dest = '\0';
    
    *len = dest - path;
    return path;
}

#define PATH_CANON_CANON1( ptr, len )           path_canon_canonicalize( ptr, (size_t *)len, 0 )
#define PATH_CANON_CANON2( ptr, len, paramflg ) path_canon_canonicalize( ptr, (size_t *)len, (int)paramflg )
#define PATH_CANON_GET_NAME( _1, _2, PATH_CANON_NAME, ... ) PATH_CANON_NAME
#define pcl_canonicalize( ptr, ... ) PATH_CANON_GET_NAME( __VA_ARGS__, PATH_CANON_CANON2, PATH_CANON_CANON1 )( ptr, __VA_ARGS__ )

#endif
