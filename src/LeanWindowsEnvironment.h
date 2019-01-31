// As lean as possible Windows.h header global settings

// Should be included as soon as possible (first line in translation unit preferably)

#ifndef COMMON_LEAN_WINDOWS_ENVIRONMENT_H
#define COMMON_LEAN_WINDOWS_ENVIRONMENT_H

#ifdef _INC_WINDOWS
	#error Some Windows.h is already included before this file!
#endif //_INC_WINDOWS


#define WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS
//#define NOVIRTUALKEYCODES // need virtual keys e.g. VK_ESCAPE
//#define NOWINMESSAGES // need window message definitions
//#define NOWINSTYLES // need window styles to create windows
#define NOSYSMETRICS
#define NOMENUS
//#define NOICONS // need icons to create window
#define NOKEYSTATES
//#define NOSYSCOMMANDS // need to manage LAlt key events
#define NORASTEROPS
//#define NOSHOWWINDOW // need to show windows
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
//#define NOCOLOR // need default colors
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
//#define NOUSER // need it - contains MSG and lot other things
//#define NONLS // need CP_UTF8
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
//#define NOMSG // need MSG for event loop
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
//#define NOWINOFFSETS // need for GetWindowLongW
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

// rip ANSI mode
#ifndef UNICODE
	#define UNICODE
#endif

#endif //COMMON_LEAN_WINDOWS_ENVIRONMENT_H