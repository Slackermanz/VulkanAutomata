#include "slack_utils.h"
#include <vector>

std::string str_p2hex( void *val ) {
	std::stringstream s;
		s << std::hex << val;
	return s.str(); }

std::string str_inplace( std::string s, std::string f, std::string r ) {
	if( r.find( f ) != -1 ) { return s; }
	else {
		int 	index  =  0;
		while ( index != -1 ) {
				index  = s.find( f );
			if( index != -1 ) { s.replace( index, f.length(), r ); } }
		return s; } }

std::string str_charray( void* val, int len ) {
	return std::string((const char*)val, len); }

std::string str_charray( const void* val, int len ) {
	return std::string((const char*)val, len); }

std::string str_pad( uint32_t val, uint32_t len, std::string chr ){
	std::string s = std::to_string(val);
	if( s.size() >= len ) { return s; }
	else {
		for(int i = s.size(); i < len; i++) { s = chr + s; }
		return s; } }

std::string str_pad( std::string val, uint32_t len, std::string chr ){
	if( val.size() >= len ) { return val; }
	else {
		for(int i = val.size(); i < len; i++) { val = chr + val; }
		return val; } }

uint32_t get_bit( uint32_t u32, uint32_t off ) 	{ return !((u32 ^ (1u << off))&(1u << off)) ? 1u : 0u; }

uint32_t get_wrd( uint32_t u32, uint32_t off, uint32_t len ) {
	uint32_t val = 0u;
	uint32_t exp = 1u;
	len = len - (((off+len)/32u) * ((off+len)%32u));
	for(int i = off; i < off+len; i++) { val += get_bit(u32,i) * exp; exp *= 2u; }
	return val; }

uint32_t u32_flp( uint32_t u32, uint32_t off ) 	{ return u32 	^ (1u << off); }
uint32_t u32_set( uint32_t u32, uint32_t off ) 	{ return u32 	| (1u << off); }
uint32_t u32_clr( uint32_t u32, uint32_t off ) 	{ return u32 	& (1u << off); }

void set_rand( uint32_t v ) { srand(v); }

uint32_t u32_rnd( uint32_t len ) { 
//	uint32_t val = 0u;
//	uint32_t exp = 1u;
//	for(int i = 0; i < len; i++) { val += (rand() & 1) * exp; exp *= 2u; }
	uint32_t val = rand()%256u
				 + rand()%256u * 256u
				 + rand()%256u * 65536u
				 + rand()%256u * 16777216u;
	return val; }

uint32_t wrd_clr( uint32_t u32, uint32_t off, uint32_t len ) {
	len = len - (((off+len)/32u) * ((off+len)%32u));
	for(int i = off; i < off+len; i++) { u32 = u32_set(u32, i); }
	return u32; }

uint32_t wrd_set( uint32_t u32, uint32_t off, uint32_t len ) {
	len = len - (((off+len)/32u) * ((off+len)%32u));
	for(int i = off; i < off+len; i++) { u32 = u32_set(u32, i); }
	return u32; }

uint32_t wrd_flp( uint32_t u32, uint32_t off, uint32_t len ) {
	len = len - (((off+len)/32u) * ((off+len)%32u));
	for(int i = off; i < off+len; i++) { u32 = u32_flp(u32, i); }
	return u32; }

uint32_t u32_mut( uint32_t u32, uint32_t rnd ) { 
	for(int i = 0; i < 32; i++) { if(rand()%rnd == 0u) { u32 = u32_flp(u32, i); } }
	if(rand()%(rnd*2u) == 0u) { u32 = wrd_set( u32, rand()%32u, rand()%8u ); }
	if(rand()%(rnd*2u) == 0u) { u32 = wrd_clr( u32, rand()%32u, rand()%8u ); }
	if(rand()%(rnd*2u) == 0u) { u32 = wrd_flp( u32, rand()%32u, rand()%8u ); }
	return u32; }
