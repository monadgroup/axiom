#pragma once

#include <utility>

namespace AxiomCommon {

    template<class Return, class Data, class... Args>
    class NamedLambda {
    public:
        using FunctionPointer = Return (*)(Data &, Args...);

        Data data;
        FunctionPointer func;

        NamedLambda(Data data, FunctionPointer func) : data(std::move(data)), func(func) {}

        Return operator()(Args &&... args) { return func(data, args...); }
    };

    template<class Output, class Input, class Data>
    using MapLambda = NamedLambda<Output, Data, Input>;

    template<class Value, class Data>
    using FilterLambda = NamedLambda<bool, Data, Value &>;
}
