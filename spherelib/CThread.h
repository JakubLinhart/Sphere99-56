#pragma once

#ifdef _WIN32
#define THREAD_ENTRY_RET void
#else	// else LINUX
#define THREAD_ENTRY_RET void *
#endif

class CThreadLockableObj
{
};

class CThreadLockPtr
{
public:
	CThreadLockPtr(CThreadLockableObj* pLockThis) { throw "not implemented"; }
	CThreadLockPtr() { throw "not implemented"; }
};

class CThread	// basic multi tasking functionality.
{
};