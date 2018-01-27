#pragma once

#include <sstream>

#include "../SourcePos.h"

namespace MaximAst {

    class Expression {
    public:
        SourcePos startPos;
        SourcePos endPos;

        Expression(SourcePos startPos, SourcePos endPos) : startPos(startPos), endPos(endPos) {}

        virtual ~Expression() = 0;

        virtual void appendString(std::stringstream &s) = 0;
    };

}
