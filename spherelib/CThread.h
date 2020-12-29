#pragma once

#ifdef _WIN32
#define THREAD_ENTRY_RET void
#else	// else LINUX
#define THREAD_ENTRY_RET void *
#endif
class CThread	// basic multi tasking functionality.
{
};