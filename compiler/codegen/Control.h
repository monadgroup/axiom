#pragma once

#include "../common/ControlType.h"
#include "Instantiable.h"

namespace MaximCodegen {

    class MaximContext;

    class Value;

    class Control : public Instantiable {
    public:
        enum class Direction {
            NONE,
            IN,
            OUT
        };

        Direction direction = Direction::NONE;

        Control(MaximContext *context, MaximCommon::ControlType type);

        MaximCommon::ControlType type() const { return _type; }

        virtual bool validateProperty(std::string name) = 0;

        virtual void setProperty(Builder &b, std::string name, std::unique_ptr<Value> val, llvm::Value *ptr) = 0;

        virtual std::unique_ptr<Value> getProperty(Builder &b, std::string name, llvm::Value *ptr) = 0;

    protected:
        MaximContext *context() const { return _context; }

    private:
        MaximContext *_context;
        MaximCommon::ControlType _type;
    };

}
