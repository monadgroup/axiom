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
        return promise;
    }

    template<class OutputType, class InputType>
    std::shared_ptr<AxiomCommon::Promise<OutputType>>
        chain(AxiomCommon::Promise<InputType> *input,
              std::function<std::shared_ptr<AxiomCommon::Promise<OutputType>>(InputType &)> callback) {
        auto output = std::make_shared<AxiomCommon::Promise<OutputType>>();

        input->then([output, callback](InputType &input) mutable {
            callback(input)->then([output](OutputType &result) mutable { output->resolve(result); });
        });

        return output;
    };

    template<class OutputType, class InputType>
    std::shared_ptr<AxiomCommon::Promise<OutputType>> chain(AxiomCommon::Promise<InputType> *input,
                                                            std::function<OutputType(InputType &)> callback) {
        return chain(input, std::function<std::shared_ptr<AxiomCommon::Promise<OutputType>>(InputType &)>(
                                [callback](InputType &v) {
                                    return std::make_shared<AxiomCommon::Promise<OutputType>>(from(callback(v)));
                                }));
    };

    template<class FirstPromise>
    std::shared_ptr<AxiomCommon::Promise<std::tuple<FirstPromise>>>
        all(std::shared_ptr<AxiomCommon::Promise<FirstPromise>> firstPromise) {
        return chain(firstPromise.get(), std::function<std::tuple<FirstPromise>(FirstPromise &)>([](FirstPromise &val) {
                         FirstPromise v = val;
                         return std::make_tuple<FirstPromise>(std::move(v));
                     }));
    }

    template<class FirstPromise, class... PromiseArgs>
    std::shared_ptr<AxiomCommon::Promise<std::tuple<FirstPromise, PromiseArgs...>>>
        all(std::shared_ptr<AxiomCommon::Promise<FirstPromise>> firstPromise,
            std::shared_ptr<AxiomCommon::Promise<PromiseArgs>>... promises) {
        return chain(firstPromise.get(),
                     std::function<std::shared_ptr<AxiomCommon::Promise<std::tuple<FirstPromise, PromiseArgs...>>>(
                         FirstPromise &)>([promises...](FirstPromise &val) {
                         return chain(
                             all(promises...).get(),
                             std::function<std::tuple<FirstPromise, PromiseArgs...>(std::tuple<PromiseArgs...> &)>(
                                 [val](std::tuple<PromiseArgs...> &otherVals) mutable {
                                     return std::tuple_cat(std::make_tuple<FirstPromise>(std::move(val)), otherVals);
                                 }));
                     }));
    }
}
