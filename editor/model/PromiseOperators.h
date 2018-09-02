#pragma once

#include <tuple>

#include "common/Promise.h"

namespace AxiomModel {

    template<class Type>
    AxiomCommon::Promise<Type> from(AxiomCommon::Promise<Type> val) {
        return std::move(val);
    }

    template<class Type>
    AxiomCommon::Promise<Type> from(Type val) {
        AxiomCommon::Promise<Type> promise;
        promise.resolve(std::move(val));
        return std::move(promise);
    }

    template<class OutputType, class InputType>
    AxiomCommon::Promise<OutputType> chain(AxiomCommon::Promise<InputType> input,
                                           std::function<AxiomCommon::Promise<OutputType>(InputType &)> callback) {
        AxiomCommon::Promise<OutputType> output;

        input.then([output, callback](InputType &input) mutable {
            callback(input).then([output](OutputType &result) mutable { output.resolve(result); });
        });

        return std::move(output);
    };

    template<class OutputType, class InputType>
    AxiomCommon::Promise<OutputType> chain(AxiomCommon::Promise<InputType> input,
                                           std::function<OutputType(InputType &)> callback) {
        return chain(std::move(input), std::function<AxiomCommon::Promise<OutputType>(InputType &)>([callback](InputType &v) { return from(callback(v)); }));
    };

    template<class FirstPromise>
    AxiomCommon::Promise<std::tuple<FirstPromise>> all(AxiomCommon::Promise<FirstPromise> firstPromise) {
        return chain(std::move(firstPromise), std::function<std::tuple<FirstPromise>(FirstPromise &)>([](FirstPromise &val) {
                         FirstPromise v = val;
                         return std::make_tuple<FirstPromise>(std::move(v));
                     }));
    }

    template<class FirstPromise, class... PromiseArgs>
    AxiomCommon::Promise<std::tuple<FirstPromise, PromiseArgs...>> all(AxiomCommon::Promise<FirstPromise> firstPromise,
                                                                       AxiomCommon::Promise<PromiseArgs>... promises) {
        return chain(std::move(firstPromise), std::function<AxiomCommon::Promise<std::tuple<FirstPromise, PromiseArgs...>>(FirstPromise &)>([promises...](FirstPromise &val) {
                         return chain(all(std::move(promises)...),
                                      std::function<std::tuple<FirstPromise, PromiseArgs...>(std::tuple<PromiseArgs...> &)>([val](std::tuple<PromiseArgs...> &otherVals) mutable {
                                          return std::tuple_cat(std::make_tuple<FirstPromise>(std::move(val)),
                                                                otherVals);
                                      }));
                     }));
    }
}
