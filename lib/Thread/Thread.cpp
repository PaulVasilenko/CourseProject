#include "Thread.h"

void Thread::Join() {
    pthread_join(mThread, NULL);
}

void Thread::Start() {
    pthread_create(&mThread, 0, &ThreadRunner, this);
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