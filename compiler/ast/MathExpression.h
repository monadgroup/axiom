#pragma once

#include "Expression.h"

namespace MaximAst {

    class MathExpression : public Expression {
    public:
        enum class Type {
            ADD,
            SUBTRACT,
            MULTIPLY,
            DIVIDE,
            MODULO,
            POWER,

            BITWISE_AND,
            BITWISE_OR,
            BITWISE_XOR,
            LOGICAL_AND,
            LOGICAL_OR,
            LOGICAL_EQUAL,
            LOGICAL_NOT_EQUAL,
            LOGICAL_GT,
            LOGICAL_LT,
            LOGICAL_GTE,
            LOGICAL_LTE
        };

        std::unique_ptr<Expression> left;
        Type type;
        std::unique_ptr<Expression> right;

        MathExpression(std::unique_ptr<Expression> left, Type type, std::unique_ptr<Expression> right, SourcePos start,
                       SourcePos end)
                : Expression(start, end), left(std::move(left)), type(type), right(std::move(right)) {}

        void appendString(std::stringstream &s) override {
            s << "(math ";
            switch (type) {
                case Type::ADD:
                    s << "+";
                    break;
                case Type::SUBTRACT:
                    s << "-";
                    break;
                case Type::MULTIPLY:
                    s << "*";
                    break;
                case Type::DIVIDE:
                    s << "/";
                    break;
                case Type::MODULO:
                    s << "%";
                    break;
                case Type::POWER:
                    s << "^";
                    break;
                case Type::BITWISE_AND:
                    s << "&";
                    break;
                case Type::BITWISE_OR:
                    s << "|";
                    break;
                case Type::BITWISE_XOR:
                    s << "^^";
                    break;
                case Type::LOGICAL_AND:
                    s << "&&";
                    break;
                case Type::LOGICAL_OR:
                    s << "||";
                    break;
                case Type::LOGICAL_EQUAL:
                    s << "==";
                    break;
                case Type::LOGICAL_NOT_EQUAL:
                    s << "!=";
                    break;
                case Type::LOGICAL_GT:
                    s << ">";
                    break;
                case Type::LOGICAL_LT:
                    s << "<";
                    break;
                case Type::LOGICAL_GTE:
                    s << ">=";
                    break;
                case Type::LOGICAL_LTE:
                    s << "<=";
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
