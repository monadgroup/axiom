#pragma once

#include <utility>
#include <vector>
#include <memory>

#include "Expression.h"
#include "../common/FormType.h"

namespace MaximAst {

    class Form {
    public:
        MaximCommon::FormType type;

        SourcePos startPos;
        SourcePos endPos;

        Form(MaximCommon::FormType type, SourcePos start, SourcePos end)
            : type(type), startPos(start), endPos(end) {}

        void appendString(std::ostream &s) {
            s << "(form " << MaximCommon::formType2String(type) << ")";
        }
    };

}
