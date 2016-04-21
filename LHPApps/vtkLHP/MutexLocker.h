#ifndef WINDOWS_API_H_
#define WINDOWS_API_H_
#pragma once

#include "vtkMutexLock.h"


/**
* Safe mutex locker. When placed on stack,
* unlock mutex if destructed => even if exception thrown.
*/
struct MutexLocker {
public:
	MutexLocker(vtkMutexLock &mutex) {
		this->mutex = &mutex;
		
		this->Relock();
	}

	MutexLocker(vtkMutexLock *mutex) {
		this->mutex = mutex;
		this->Relock();
	}

	~MutexLocker() {
		this->Unlock();
	}

	void Relock() {
		this->mutex->Lock();
	}

	void Unlock() {
		this->mutex->Unlock();
	}
private:
	vtkMutexLock* mutex;
	
};


#endif