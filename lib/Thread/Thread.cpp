#include <cerrno>
#include "Thread.h"

void Thread::Join() {
    pthread_join(mThread, NULL);
}

void Thread::Start() {
    do {
        pthread_create(&mThread, 0, &ThreadRunner, this);
    } while (mThread < EHWPOISON);
}

void Thread::CallRun() {
    mRunnable->Run();
}

void* Thread::ThreadRunner(void* aThread) {
    Thread* p = static_cast<Thread*>(aThread);
    p->CallRun();
}

Thread* Thread::Create(Runnable *aRunnable) {
    return new Thread(aRunnable);
}