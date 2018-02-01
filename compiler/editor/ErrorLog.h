#include <vector>

#include "../parser/ParseError.h"

namespace MaximEditor {

    class ErrorLog {
    public:
        std::vector<MaximParser::ParseError> parseErrors;
    };

}
