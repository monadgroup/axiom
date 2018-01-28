#pragma once

#include "Expression.h"
#include "LValueExpression.h"

namespace MaximAst {

    class AssignExpression : public Expression {
    public:
        enum class Type {
            ASSIGN,
            ADD,
            SUBTRACT,
            MULTIPLY,
            DIVIDE,
            MODULO,
            POWER
        };

        std::unique_ptr<LValueExpression> left;
        Type type;
        std::unique_ptr<Expression> right;

        AssignExpression(std::unique_ptr<LValueExpression> left, Type type, std::unique_ptr<Expression> right,
                         SourcePos start, SourcePos end)
                : Expression(start, end), left(std::move(left)), type(type), right(std::move(right)) {}

        void appendString(std::ostream &s) override {
            s << "(assign ";
            switch (type) {
                case Type::ASSIGN:
                    s << "=";
                    break;
                case Type::ADD:
                    s << "+=";
                    break;
                case Type::SUBTRACT:
                    s << "-=";
                    break;
                case Type::MULTIPLY:
                    s << "*=";
                    break;
                case Type::DIVIDE:
                    s << "/=";
                    break;
                case Type::MODULO:
                    s << "%=";
                    break;
                case Type::POWER:
                    s << "^=";
                    break;
            }
            s << " ";
            left->appendString(s);
            s << " ";
            right->appendString(s);
            s << ")";
        }
    };

}
