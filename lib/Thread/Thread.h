//
// Created by user on 19.4.16.
//

#ifndef COURSEPROJECTSERVER_THREAD_H
#define COURSEPROJECTSERVER_THREAD_H


#include "Runnable.h"
#include "pthread.h"

class Thread {
public:
    static void* ThreadRunner(void* aThread);
    static Thread* Create(Runnable* aRunnable);
    virtual ~Thread() {}
    virtual void Join();
    virtual void Start();
    virtual void CallRun();
protected:
    Thread(Runnable* aRunnable)
            : mRunnable(aRunnable) {}

    Runnable* mRunnable;
private:
    pthread_t mThread;
};


#endif //COURSEPROJECTSERVER_THREAD_H
