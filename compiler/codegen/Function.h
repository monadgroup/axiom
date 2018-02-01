#pragma once

#include <vector>
#include <memory>

#include "Parameter.h"

namespace MaximCodegen {

    class TupleType;

    class Function {
    public:
        Function(bool isPure, std::unique_ptr<TupleType> returnType, std::vector<Parameter> parameters,
                 std::unique_ptr<Parameter> variadicParam);

        bool isPure() const { return _isPure; }

        TupleType *returnType() const { return _returnType.get(); }

        std::vector<Parameter> const &parameters() const { return _parameters; }

        Parameter *variadicParam() const { return _variadicParam.get(); }

    private:
        bool _isPure;
        std::unique_ptr<TupleType> _returnType;
        std::vector<Parameter> _parameters;
        std::unique_ptr<Parameter> _variadicParam;
    };

}
