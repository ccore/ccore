#include "win_thread.h"

#ifdef CC_USE_THREAD

ccReturn ccThreadStart(ccThread *thread, void *function, void *data)
{
	*thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)function, (LPVOID)data, 0, NULL);
	if(*thread == NULL) {
		ccErrorPush(CC_ERROR_THREAD_CREATE);
		return CC_FAIL;
	}
	return CC_SUCCESS;
}

ccReturn ccThreadJoin(ccThread *thread)
{
	if(WaitForSingleObject(*thread, INFINITE) == WAIT_OBJECT_0) {
		if(CloseHandle(*thread) == 0) {
			ccErrorPush(CC_ERROR_THREAD_CREATE);
			return CC_FAIL;
		}
		return CC_SUCCESS;
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

ccReturn ccThreadMutexCreate(ccMutex *mutex)
{
	if(!InitializeCriticalSectionAndSpinCount(mutex, _CRITICAL_SECTION_SPIN_COUNT)) {
		ccErrorPush(CC_ERROR_THREAD_MUTEXCREATE);
		return CC_FAIL;
	}
	
	return CC_SUCCESS;
}

ccReturn ccThreadMutexJoin(ccMutex *mutex)
{
	EnterCriticalSection(mutex);

	return CC_SUCCESS;
}

ccReturn ccThreadMutexRelease(ccMutex *mutex)
{
	LeaveCriticalSection(mutex);

	return CC_SUCCESS;
}

ccReturn ccThreadMutexFree(ccMutex *mutex)
{
	DeleteCriticalSection(mutex);

	return CC_SUCCESS;
}

#endif