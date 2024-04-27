#include "mutex.h"

#if defined(CEL_PLATFORM_LINUX) || defined(CEL_PLATFORM_MAC)
// TODO: implement in terms of pthreads
#endif

#if defined(CEL_PLATFORM_WINDOWS)
// TODO: implement using win32 api
#endif