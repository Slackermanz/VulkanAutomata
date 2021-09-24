#include "slack_utils.h"

std::string str_p2hex( void *val ) {
	std::stringstream s;
		s << std::hex << long(val);
	return "0x"+s.str(); }

std::string str_inplace( std::string s, std::string f, std::string r ) {
	if( r.find( f ) != -1 ) { return s; }
	else {
		int 	index  =  0;
		while ( index != -1 ) {
				index  = s.find( f );
			if( index != -1 ) { s.replace( index, f.length(), r ); } }
		return s; } }

std::string str_charray( void* val, int len ) {
	char s[len];
	memcpy( &s, val, len );
	return s; }
