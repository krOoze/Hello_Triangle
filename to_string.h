#ifndef COMMON_TO_STRING_WORKAROUND_H
#define COMMON_TO_STRING_WORKAROUND_H

#include <string>
#include <sstream>

#include "CompilerMessages.h"

TODO( "These are workaround functions for the broken MinGW" );
#ifdef __MINGW32__

template<typename T>
string to_string_via_ss( T i ){
	std::ostringstream ss;
	ss << i;
	return ss.str();
}

template<typename T>
string to_string( T i ){ return to_string_via_ss( i ); }

#else
using std::to_string;
#endif

#endif // COMMON_TO_STRING_WORKAROUND_H
