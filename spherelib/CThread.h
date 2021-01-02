#pragma once

#ifdef _WIN32
#define THREAD_ENTRY_RET void
#else	// else LINUX
#define THREAD_ENTRY_RET void *
#endif

typedef THREAD_ENTRY_RET(_cdecl* PTHREAD_ENTRY_PROC)(void*);

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
public:
	static DWORD GetCurrentThreadId() { throw "not implemented"; }

	void CreateThread(PTHREAD_ENTRY_PROC pEntryProc) { throw "not implemented"; }
	void CreateThread(PTHREAD_ENTRY_PROC pEntryProc, void* pArgs) { throw "not implemented"; }

};