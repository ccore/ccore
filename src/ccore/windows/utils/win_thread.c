#include "win_thread.h"

#if defined CC_USE_ALL || defined CC_USE_THREAD

ccError ccThreadStart(ccThread *thread, void *function, void *data)
{
	*thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)function, (LPVOID)data, 0, NULL);
	if(*thread == NULL) {
		ccErrorPush(CC_ERROR_THREAD_CREATE);
		return CC_FAIL;
	}
	return CC_E_NONE;
}

ccError ccThreadJoin(ccThread *thread)
{
	if(WaitForSingleObject(*thread, INFINITE) == WAIT_OBJECT_0) {
		if(CloseHandle(*thread) == 0) {
			ccErrorPush(CC_ERROR_THREAD_CREATE);
			return CC_FAIL;
		}
		return CC_E_NONE;
	}
	else{
		ccErrorPush(CC_ERROR_THREAD_CREATE);
		return CC_FAIL;
	}
}

bool ccThreadFinished(ccThread *thread)
{
	if(WaitForSingleObject(*thread, 0) == WAIT_OBJECT_0) {
		if(CloseHandle(*thread) == 0) {
			ccErrorPush(CC_ERROR_THREAD_CREATE);
		}
		return true;
	}

	return false;
}

ccError ccThreadMutexCreate(ccMutex *mutex, unsigned int spinCount)
{
	if(!InitializeCriticalSectionAndSpinCount(mutex, spinCount)) {
		ccErrorPush(CC_ERROR_THREAD_MUTEXCREATE);
		return CC_FAIL;
	}
	
	return CC_E_NONE;
}

ccError ccThreadMutexJoin(ccMutex *mutex)
{
	EnterCriticalSection(mutex);

	return CC_E_NONE;
}

ccError ccThreadMutexRelease(ccMutex *mutex)
{
	LeaveCriticalSection(mutex);

	return CC_E_NONE;
}

ccError ccThreadMutexFree(ccMutex *mutex)
{
	DeleteCriticalSection(mutex);

	return CC_E_NONE;
}

#endif