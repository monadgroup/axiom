#pragma once

#include <memory>

namespace AxiomUtil {

    template<class To, class From>
    std::unique_ptr<To> dynamic_unique_cast(std::unique_ptr<From> &from) {
        if (To *result = dynamic_cast<To *>(from.get())) {
            from.release();
            return std::unique_ptr<To>(result);
        }
        return std::unique_ptr<To>(nullptr);
    };

}
