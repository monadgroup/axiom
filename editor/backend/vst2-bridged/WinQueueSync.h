#pragma once

#include <string>

namespace AxiomBackend {

    class QueueSync {
    public:
        class SeparateData {
        public:
            void *mutex;
            void *event;

            explicit SeparateData(const std::string &id);
            SeparateData(const SeparateData &) = delete;
            SeparateData(SeparateData &&) = delete;
            SeparateData &operator=(const SeparateData &) = delete;
            SeparateData &operator=(SeparateData &&) = delete;
            ~SeparateData();
        };

        QueueSync() = default;
        QueueSync(const QueueSync &) = delete;
        QueueSync(QueueSync &&) = delete;
        QueueSync &operator=(const QueueSync &) = delete;
        QueueSync &operator=(QueueSync &&) = delete;

        void lock(SeparateData &sep);
        void unlock(SeparateData &sep);
        void wait(SeparateData &sep);
        void wakeOne(SeparateData &sep);
    };

}
