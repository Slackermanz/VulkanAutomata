#include "slack_debug.h"

void ol( std::string msg ) { std::cout << msg << "\n"; }
void oh( std::string msg ) { ol("____________________________________________________________\n " + msg ); }

void ov( std::string msg, std::string val, int idx ) {
	std::string hed 	= "    info:   ";
	std::string pad 	= " ";
	std::string sid		= " ";
	int 		pads	= 12;
	if( idx != 2147483647 ) {
		sid = " " + std::to_string(idx) + " ";
		for(int i = 0; i < 13-sid.size(); i++) { sid = " " + sid; }	}
	int 		padsize = (pads*4 - msg.size() - sid.size()) - 1;
	for(int i = 0; i < padsize; i++) { pad = pad + "."; }
	ol(hed + msg + pad + sid + "[" + val + "]" ); }

void ov( std::string msg 							) 	{ oh( msg 								); }
void ov( std::string msg, int 		val, 	int idx ) 	{ ov( msg, std::to_string(val), 	idx ); }
void ov( std::string msg, uint32_t 	val, 	int idx ) 	{ ov( msg, std::to_string(val), 	idx ); }
void ov( std::string msg, float 	val, 	int idx ) 	{ ov( msg, std::to_string(val), 	idx ); }
void ov( std::string msg, double 	val, 	int idx ) 	{ ov( msg, std::to_string(val), 	idx ); }
void ov( std::string msg, void	   *val, 	int idx ) 	{ ov( msg, str_p2hex(val), 			idx ); }
void rv( std::string msg, 				 	int idx ) 	{ ov( msg, "void", 					idx ); }
void av( std::string msg, void* 	val, 	int idx ) 	{ ov( msg, str_charray(val, 48), 	idx ); }
