#include "lin_thread.h"

#if defined CC_USE_ALL || defined CC_USE_THREAD

ccReturn ccThreadStart(ccThread *thread, void *function, void *data)
{
	if(CC_UNLIKELY(pthread_create(thread, NULL, function, data) != 0)) {
		ccErrorPush(CC_ERROR_THREAD_CREATE);
		return CC_FAIL;
	}

	return CC_SUCCESS;
}

ccReturn ccThreadJoin(ccThread *thread)
{
	if(CC_UNLIKELY(pthread_join(*thread, NULL) == 0)){
		return CC_SUCCESS;  
	}else{
		ccErrorPush(CC_ERROR_THREAD_CREATE);
		return CC_FAIL;
	}
}

bool ccThreadFinished(ccThread *thread)
{
	if(pthread_kill(*thread, 0) == 0){
		return false;
	}else{
		return true;
	}
}

ccReturn ccThreadMutexCreate(ccMutex *mutex)
{
	if(CC_UNLIKELY(pthread_mutex_init(mutex, NULL) != 0)){
		ccErrorPush(CC_ERROR_THREAD_MUTEXCREATE);
		return CC_FAIL;
	}

	return CC_SUCCESS;
}

ccReturn ccThreadMutexJoin(ccMutex *mutex)
{
	if(CC_UNLIKELY(pthread_mutex_lock(mutex) != 0)) {
		ccErrorPush(CC_ERROR_THREAD_MUTEX);
		return CC_FAIL;
	}

	return CC_SUCCESS;
}

ccReturn ccThreadMutexRelease(ccMutex *mutex)
{
	if(CC_UNLIKELY(pthread_mutex_unlock(mutex) != 0)){
		ccErrorPush(CC_ERROR_THREAD_MUTEX);
		return CC_FAIL;
	}

	return CC_SUCCESS;
}

ccReturn ccThreadMutexFree(ccMutex *mutex)
{
	if(CC_UNLIKELY(pthread_mutex_destroy(mutex) != 0)){
		ccErrorPush(CC_ERROR_THREAD_MUTEX);
		return CC_FAIL;
	}

	return CC_SUCCESS;
}

#endif