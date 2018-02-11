#pragma once

namespace MaximCodegen {

    class Converter {
    public:
        std::unique_ptr<Num> call(std::unique_ptr<Num> value);
    };

}
