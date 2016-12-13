# PathCanonicalizer

Path Canonicalize library.
***


## Uses
* Remove relative path from path.  
* Change relative path to absolute path.  
* Format validation for URL encoded UTF-8 format.
* Reject not avarable charactors for path.

### Remove relative path from path
Remove "./" , "../" and "/..".  
http://www.dmm.com/hoge/./../index.html  
(scheme)://(host)/(path)  
> foo/bar/./../index.html/..  
> => /foo/index.html/

### Change relative path to absolute path
Add "/" to path.
> foo  
> => /foo

### Format validation for URL encoded UTF-8 format
"あ" = "%E3%81%82"
> "%E3%81%"  
> => NULL

### Reject not ASCII charactors and path unavailable charactors.
> あ  
> => NULL  
> 
> /a?b=a  
> => NULL or /a?b=a  
> ( selectable )

***


## Interface
### static inline char* canonicalize( const char *ptr, size_t *len );
### static inline char* canonicalize( const char *ptr, size_t *len, int paramflg( default 0 ) );
Path Canonicalization method.
```
size_t len = 25;
const char *src = "foo/bar/./../%E3%81%82/..";
char *path;

if( ( path = canonicalize( src, &len ) ) != NULL ){
    printf("PATH:   %s\n", path);
    printf("LENGTH: %d\n", len);
    free( path );
    return;
}
printf("errno: %d by errno.h\n", errno);

> PATH:   /foo/%E3%81%82/  
> LENGTH: 15
```

### static inline int checkPercentUtf8( char** dest, char** ptr )
Format validation for URL encoded UTF-8 format.
```
size_t len = 25 + 1;
char *src = "foo/bar/./../%E3%81%82%E3%81%82/..";
char *path = (char *) malloc( *len ) );
char *dest = path;

while( &ptr ){
    printf( "%c", ptr );
    if( *ptr == '%' ){
        if( checkPercentUtf8( &dest, &ptr ) ){
            break;
        }
        printf( "\n" );
    }else{
        *dest = *ptr;
        dest++;
        ptr++;
    }
}
printf( "foo/bar/./../%E3%81%82%E3%81%82/..", dest );
free( dest );

> foo/bar/./../%E3%81%82
> %E3%81%82
> /..
> foo/bar/./../%E3%81%82%E3%81%82/..
```

### static inline int rfc3986PathCharactor( char chr )
Check available charactor for path. ( NOT URL e.g. '/', '#' )
```
printf( "%s, ", rfc3986UrlCharactor( 'a' ) ? "True" : "False" );
printf( "%s, ", rfc3986UrlCharactor( '1' ) ? "True" : "False" );
printf( "%s, ", rfc3986UrlCharactor( '%' ) ? "True" : "False" );
printf( "%s, ", rfc3986UrlCharactor( '/' ) ? "True" : "False" );
printf( "%s\n", rfc3986UrlCharactor( '.' ) ? "True" : "False" );
printf( "%s, ", rfc3986UrlCharactor(  1  ) ? "True" : "False" );
printf( "%s, ", rfc3986UrlCharactor( '!' ) ? "True" : "False" );
printf( "%s, ", rfc3986UrlCharactor( '#' ) ? "True" : "False" );
printf( "%s, ", rfc3986UrlCharactor( '\' ) ? "True" : "False" );
printf( "%s, ", rfc3986UrlCharactor( '*' ) ? "True" : "False" );
printf( "%s, ", rfc3986UrlCharactor( '(' ) ? "True" : "False" );
printf( "%s, ", rfc3986UrlCharactor( ')' ) ? "True" : "False" );
printf( "%s\n", rfc3986UrlCharactor( '\\' ) ? "True" : "False" );

> True, True, True, True, True
> False, False, False, False, False, False, False, False
```

## Install

    autoreconf -ivf
    ./configure
    make
    make check
    make install