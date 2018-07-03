#pragma once

namespace MaximCompiler {

    class OwnedObject {
    public:

        explicit OwnedObject(void *handle, void (*destroy)(void *));

        virtual ~OwnedObject();

        OwnedObject(OwnedObject &&) noexcept;

        OwnedObject &operator=(OwnedObject &&) noexcept;

        OwnedObject(const OwnedObject &) = delete;

        OwnedObject &operator=(const OwnedObject &) = delete;

        void *get() const { return handle; }

        void *release();

    private:
        void *handle;

        void (*destroy)(void *);
    };

}
