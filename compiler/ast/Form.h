#pragma once

#include <utility>
#include <vector>
#include <memory>

#include "Expression.h"

namespace MaximAst {

    class Form {
    public:
        std::string name;
        std::vector<std::unique_ptr<Expression>> arguments;

        SourcePos startPos;
        SourcePos endPos;

        Form(std::string name, SourcePos start, SourcePos end)
                : name(std::move(name)), startPos(start), endPos(end) { }

        void appendString(std::stringstream &s) {
            s << "(form " << name << " ";
            for (size_t i = 0; i < arguments.size(); i++) {
                arguments[i]->appendString(s);
                if (i != arguments.size() - 1) s << " ";
            }
            s << ")";
        }
    };

}
