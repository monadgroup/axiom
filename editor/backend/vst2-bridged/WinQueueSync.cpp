#include "WinQueueSync.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cassert>

using namespace AxiomBackend;

QueueSync::SeparateData::SeparateData(const std::string &id) {
    auto mutexId = id + ".mutex";
    auto eventId = id + ".event";

    mutex = CreateMutexA(nullptr, false, mutexId.c_str());
    assert(mutex);
    event = CreateEventA(nullptr, false, false, eventId.c_str());
    assert(event);
}

QueueSync::SeparateData::~SeparateData() {
    if (mutex) {
        CloseHandle(mutex);
    }
    if (event) {
        CloseHandle(event);
    }
}

void QueueSync::lock(AxiomBackend::QueueSync::SeparateData &sep) {
    auto waitResult = WaitForSingleObject(sep.mutex, INFINITE);
    assert(waitResult == WAIT_OBJECT_0 || waitResult == WAIT_ABANDONED);
}

void QueueSync::unlock(AxiomBackend::QueueSync::SeparateData &sep) {
    ReleaseMutex(sep.mutex);
}

void QueueSync::wait(AxiomBackend::QueueSync::SeparateData &sep) {
    unlock(sep);
    auto eventWaitResult = WaitForSingleObject(sep.event, INFINITE);
    assert(eventWaitResult == WAIT_OBJECT_0 || eventWaitResult == WAIT_ABANDONED);
    lock(sep);
}

void QueueSync::wakeOne(AxiomBackend::QueueSync::SeparateData &sep) {
    SetEvent(sep.event);
}
