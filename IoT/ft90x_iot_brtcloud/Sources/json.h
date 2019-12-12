/*
 * json.h
 *
 *  Created on: Dec 12, 2019
 *      Author: richmond
 */

#ifndef _IOT_JSON_H_
#define _IOT_JSON_H_

uint32_t json_parse_int( const char* ptr, char* key );
char* json_parse_str( const char* ptr, char* key, int* len );
char* json_parse_str_ex( char* ptr, char* key, char* end );


#endif /* _IOT_JSON_H_ */
