#pragma once

#include <utility>
#include <vector>
#include <memory>

#include "Expression.h"

namespace MaximAst {

    class Form {
    public:
        enum class Type {
            LINEAR,
            FREQUENCY,
            NOTE,
            DB,
            Q,
            RES,
            SECONDS,
            BEATS
        };

        Type type;
        std::vector<std::unique_ptr<Expression>> arguments;

        SourcePos startPos;
        SourcePos endPos;

        Form(Type type, SourcePos start, SourcePos end)
                : type(type), startPos(start), endPos(end) {}

        void appendString(std::ostream &s) {
            s << "(form ";
            switch (type) {
                case Type::LINEAR:
                    s << "lin";
                    break;
                case Type::FREQUENCY:
                    s << "freq";
                    break;
                case Type::NOTE:
                    s << "note";
                    break;
                case Type::DB:
                    s << "db";
                    break;
                case Type::Q:
                    s << "q";
                    break;
                case Type::RES:
                    s << "res";
                    break;
                case Type::SECONDS:
                    s << "secs";
                    break;
                case Type::BEATS:
                    s << "beats";
                    break;
            }
            if (!arguments.empty()) s << " ";
            for (size_t i = 0; i < arguments.size(); i++) {
                arguments[i]->appendString(s);
                if (i != arguments.size() - 1) s << " ";
            }
            s << ")";
        }
    };

}
