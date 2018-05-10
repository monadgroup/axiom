#pragma once

#include <functional>
#include <map>

namespace AxiomModel {

    class Hookable {
    public:
        Hookable() = default;

        Hookable(const Hookable &a) noexcept = delete;

        Hookable(Hookable &&a) noexcept;

        Hookable &operator=(Hookable &&a) noexcept;

        virtual ~Hookable();

        void addDestructHook(void *handle, std::function<void()> func);

        void removeDestructHook(void *handle);

    protected:
        virtual void triggerDestruct(bool isFinal);

    private:
        std::map<void *, std::function<void()>> destructEvents;
    };

}
