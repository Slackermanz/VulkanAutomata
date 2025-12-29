#ifndef SLACK_UTILS_H
#define SLACK_UTILS_H

	#include <string>
	#include <cstring>
	#include <sstream>

	std::string str_p2hex( void *val );
	std::string str_charray( void* val, int len );
	std::string str_inplace( std::string s, std::string f, std::string r );

#endif
