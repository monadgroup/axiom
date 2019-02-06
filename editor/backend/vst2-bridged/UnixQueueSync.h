#pragma once

#include <pthread.h>
#include <string>

namespace AxiomBackend {

    class QueueSync {
    public:
        class SeparateData {
            explicit SeparateData(const std::string &id) {}
            SeparateData(const SeparateData &) = delete;
            SeparateData(SeparateData &&) = delete;
            SeparateData &operator=(const SeparateData &) = delete;
            SeparateData &operator=(SeparateData &&) = delete;
        };

        QueueSync();
        QueueSync(const QueueSync &) = delete;
        QueueSync(QueueSync &&) = delete;
        QueueSync &operator=(const QueueSync &) = delete;
        QueueSync &operator=(QueueSync &&) = delete;
        ~QueueSync();

        void lock(SeparateData &sep);
        void unlock(SeparateData &sep);
        void wait(SeparateData &sep);
        void wakeOne(SeparateData &sep);

    private:
        pthread_mutex_t mutex;
        pthread_cond_t event;
    };

}
