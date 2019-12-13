#include "ft900.h"
#include <string.h>
#include <stdlib.h>



uint32_t json_parse_int( const char* ptr, char* key )
{
    char* start = NULL;
    char* stop = NULL;

    // the key is always followed by 3 characters "\": "
    start = strstr(ptr, key);
    if (!start) {
        return -2; // key is not found
    }
    start += strlen(key) + 3;

    // the terminator of the key value is either a comma or a close parenthesis (if last parameter)
    stop = strchr(start, ',');
    if (!stop) {
        stop = strchr(start, '}');
        if (!stop) {
            return -3; // terminator character not found
        }
    }

    return strtoul(start, &stop, 10);
}

char* json_parse_str( const char* ptr, char* key, int* len )
{
    char* start = NULL;
    char* stop = NULL;

    // the key is always followed by 4 characters "\": \""
    start = strstr(ptr, key);
    if (!start) {
        return NULL; // key is not found
    }
    start += strlen(key) + 4;

    // the terminator of the key value is a "
    stop = strchr(start, '\"');
    if (!stop) {
        return NULL; // terminator character not found
    }

    if (len) {
    	*len = stop-start;
    }
    return start;
}

char* json_parse_str_ex( char* ptr, char* key, char end )
{
    char* temp = NULL;

    temp = strchr(ptr, end);
    if (!temp) {
        return NULL;
    }
    ptr[temp-ptr] = '\0';
    ptr[temp-ptr-1] = '\0';

    temp = strstr(ptr, key);
    if (!temp) {
        return NULL;
    }
    ptr += (temp-ptr) + strlen(key);
    ptr ++;

    return ptr;
}



