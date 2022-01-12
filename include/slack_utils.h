#ifndef SLACK_UTILS_H
#define SLACK_UTILS_H

	#include <string>
	#include <cstring>
	#include <sstream>

	std::string str_p2hex( void *val );
	std::string str_charray( void* val, int len );
	std::string str_charray( const void* val, int len );
	std::string str_inplace( std::string s, std::string f, std::string r );
	std::string str_pad( uint32_t 	 val, uint32_t len = 2u, std::string chr = " " );
	std::string str_pad( std::string val, uint32_t len = 2u, std::string chr = " " );

	void set_rand( uint32_t v );

	uint32_t u32_rnd( uint32_t len = 32u );
	uint32_t get_bit( uint32_t u32, uint32_t off );
	uint32_t get_wrd( uint32_t u32, uint32_t off, uint32_t len );
	uint32_t u32_flp( uint32_t u32, uint32_t off );
	uint32_t u32_set( uint32_t u32, uint32_t off );
	uint32_t u32_clr( uint32_t u32, uint32_t off );
	uint32_t wrd_clr( uint32_t u32, uint32_t off, uint32_t len );
	uint32_t wrd_set( uint32_t u32, uint32_t off, uint32_t len );
	uint32_t wrd_flp( uint32_t u32, uint32_t off, uint32_t len );
	uint32_t u32_mut( uint32_t u32, uint32_t rnd );

#endif
