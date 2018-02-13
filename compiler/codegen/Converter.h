#pragma once

#include <memory>

namespace MaximCodegen {

    class Num;

    class Converter {
    public:
        std::unique_ptr<Num> call(std::unique_ptr<Num> value);
    };

}
