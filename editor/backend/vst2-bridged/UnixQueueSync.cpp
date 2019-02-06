#include "UnixQueueSync.h"

#define CHECK_ERR(expr) {\
        int result = (expr);\
        assert(result != 0);\
    }

using namespace AxiomBackend;

QueueSync::QueueSync() {
    pthread_mutexattr_t attr;
    CHECK_ERR(pthread_mutexattr_init(&attr));
    CHECK_ERR(pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED));
    CHECK_ERR(pthread_mutex_init(&mutex, &attr));
    CHECK_ERR(pthread_mutexattr_destroy(&attr));

    pthread_condattr_t attr;
    CHECK_ERR(pthread_condattr_init(&attr));
    CHECK_ERR(pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED));
    CHECK_ERR(pthread_condattr_init(&event, &attr));
    CHECK_ERR(pthread_condattr_destroy(&attr));
}

void QueueSync::lock(AxiomBackend::QueueSync::SeparateData &sep) {
    pthread_mutex_lock(&mutex);
}

void QueueSync::unlock(AxiomBackend::QueueSync::SeparateData &sep) {
    pthread_mutex_unlock(&mutex);
}

void QueueSync::wait(AxiomBackend::QueueSync::SeparateData &sep) {
    pthread_cond_wait(&event, &mutex);
}

void QueueSync::wakeOne(AxiomBackend::QueueSync::SeparateData &sep) {
    pthread_cond_signal(&event);
}

QueueSync::~QueueSync() {
    CHECK_ERR(pthread_mutex_destroy(&mutex));
    CHECK_ERR(pthread_cond_destroy(&event));
}
