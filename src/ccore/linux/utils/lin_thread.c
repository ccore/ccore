#include "lin_thread.h"

#if defined CC_USE_ALL || defined CC_USE_THREAD

ccError ccThreadStart(ccThread *thread, void *function, void *data)
{
	if(CC_UNLIKELY(pthread_create(thread, NULL, function, data) != 0)) {
		return CC_E_THREAD_CREATE;
	}

	return CC_E_NONE;
}

ccError ccThreadJoin(ccThread *thread)
{
	if(CC_UNLIKELY(pthread_join(*thread, NULL) == 0)){
		return CC_E_NONE;  
	}else{
		return CC_E_THREAD_CREATE;
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

ccError ccThreadMutexCreate(ccMutex *mutex, unsigned int spinCount)
{
	if(CC_UNLIKELY(pthread_mutex_init(mutex, NULL) != 0)){
		return CC_E_THREAD_MUTEXCREATE;
	}

	return CC_E_NONE;
}

ccError ccThreadMutexJoin(ccMutex *mutex)
{
	if(CC_UNLIKELY(pthread_mutex_lock(mutex) != 0)) {
		return CC_E_THREAD_MUTEX;
	}

	return CC_E_NONE;
}

ccError ccThreadMutexRelease(ccMutex *mutex)
{
	if(CC_UNLIKELY(pthread_mutex_unlock(mutex) != 0)){
		return CC_E_THREAD_MUTEX;
	}

	return CC_E_NONE;
}

ccError ccThreadMutexFree(ccMutex *mutex)
{
	if(CC_UNLIKELY(pthread_mutex_destroy(mutex) != 0)){
		return CC_E_THREAD_MUTEX;
	}

	return CC_E_NONE;
}

#endif
