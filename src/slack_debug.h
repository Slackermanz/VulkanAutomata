#ifndef SLACK_DEBUG_H
#define SLACK_DEBUG_H

	#include <iostream>
	#include "slack_utils.h"

	void ov( std::string msg );
	void ov( std::string msg, std::string 	val, int idx = 2147483647 );
	void ov( std::string msg, void* 		val, int idx = 2147483647 );
	void ov( std::string msg, int 			val, int idx = 2147483647 );
	void ov( std::string msg, uint32_t 		val, int idx = 2147483647 );
	void ov( std::string msg, float 		val, int idx = 2147483647 );
	void ov( std::string msg, double 		val, int idx = 2147483647 );
	void rv( std::string msg, 				 	 int idx = 2147483647 );
	void av( std::string msg, void* 		val, int idx = 2147483647 );

#endif
