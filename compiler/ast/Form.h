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
            CONTROL,
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
            s << "(form " << typeToString(type);
            if (!arguments.empty()) s << " ";
            for (size_t i = 0; i < arguments.size(); i++) {
                arguments[i]->appendString(s);
                if (i != arguments.size() - 1) s << " ";
            }
            s << ")";
        }

        static std::string typeToString(Type type) {
            switch (type) {
                case Type::LINEAR: return "lin";
                case Type::CONTROL: return "control";
                case Type::FREQUENCY: return "freq";
                case Type::NOTE: return "note";
                case Type::DB: return "db";
                case Type::Q: return "q";
                case Type::RES: return "res";
                case Type::SECONDS: return "secs";
                case Type::BEATS: return "beats";
            }
        }
    };

}
