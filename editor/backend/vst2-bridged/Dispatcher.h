#pragma once

#include <functional>
#include <optional>

#include "SharedQueue.h"

namespace AxiomBackend {

    enum class DispatcherHandlerResult {
        CONTINUE,
        EXIT
    };

    template<class Queue>
    class Dispatcher {
    public:
        using value_type = typename Queue::value_type;

        template<class Cb>
        Dispatcher(Queue &messageQueue, Cb handler) : messageQueue(messageQueue), handler(std::move(handler)) {}

        void run() {
            DispatcherHandlerResult lastResult;
            do {
                auto nextMessage = messageQueue.popWhenAvailable();
                lastResult = processMessage(nextMessage);
            } while (lastResult != DispatcherHandlerResult::EXIT);
        }

        DispatcherHandlerResult idle() {
            while (auto nextMessage = messageQueue.tryPop()) {
                auto processResult = processMessage(*nextMessage);

                if (processResult == DispatcherHandlerResult::EXIT) {
                    return processResult;
                }
            }

            return DispatcherHandlerResult::CONTINUE;
        }

        value_type waitForNext() {
            auto message = messageQueue.popWhenAvailable();
            auto processResult = processMessage(message);
            if (processResult == DispatcherHandlerResult::EXIT) {
                hasExited = true;
            }
            return message;
        }

        void exit() {
            hasExited = true;
        }

    private:
        Queue &messageQueue;
        std::function<DispatcherHandlerResult(const value_type &)> handler;
        bool hasExited = false;

        DispatcherHandlerResult processMessage(const value_type &message) {
            auto handlerResult = handler(message);

            if (hasExited) {
                hasExited = false;
                return DispatcherHandlerResult::EXIT;
            } else {
                return handlerResult;
            }
        }
    };

}
