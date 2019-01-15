// Allows to show messages during compilation -- somewhat platform dependent

#ifndef COMMON_COMPILER_MESSAGES_H
#define COMMON_COMPILER_MESSAGES_H


// compiler messages -- should work in MSVC and GCC and is ignored by unsupporting compilers
#define STRINGIZE_HELPER(x) #x
#define STRINGIZE(x) STRINGIZE_HELPER(x)

#ifdef NO_TODO
	#define TODO(desc) 
#else

#if defined(_MSC_VER)
	#define TODO(desc) __pragma( message(__FILE__ "(" STRINGIZE(__LINE__) ")" ": warning T: TODO: "  desc) )
#elif defined(__GNUG__)
	#define DO_PRAGMA(x) _Pragma (#x)
	#define TODO(desc) DO_PRAGMA(message "TODO: " desc)
#else
	#define TODO(desc)
#endif

#endif // NO_TODO

// usage:
//TODO("Don't forget to do X.")

#endif //COMMON_COMPILER_MESSAGES_H