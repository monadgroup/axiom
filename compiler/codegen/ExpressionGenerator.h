#pragma once

#include <memory>

#include "../ast/MathExpression.h"
#include "../ast/AssignExpression.h"

namespace MaximAst {
    class Expression;

    class AssignableExpression;

    class AssignExpression;

    class CallExpression;

    class CastExpression;

    class ControlExpression;

    class MathExpression;

    class NoteExpression;

    class NumberExpression;

    class PostfixExpression;

    class TupleExpression;

    class UnaryExpression;

    class VariableExpression;
}

namespace llvm {
    class StructType;

    class Value;
}

namespace MaximCodegen {

    class Function;

    class Scope;

    class Context;

    class Value;

    class NumValue;

    class MidiValue;

    class TupleValue;

    class ExpressionGenerator {
    public:
        explicit ExpressionGenerator(Context *context);

        std::unique_ptr<Value> generateExpr(MaximAst::Expression *expr, Function *function, Scope *scope);

    private:
        Context *_context;

        // primitives
        std::unique_ptr<Value> generateNote(MaximAst::NoteExpression *expr, Function *function, Scope *scope);

        std::unique_ptr<Value> generateNumber(MaximAst::NumberExpression *expr, Function *function, Scope *scope);

        std::unique_ptr<Value> generateTuple(MaximAst::TupleExpression *expr, Function *function, Scope *scope);

        // references
        std::unique_ptr<Value> generateCall(MaximAst::CallExpression *expr, Function *function, Scope *scope);

        std::unique_ptr<Value> generateCast(MaximAst::CastExpression *expr, Function *function, Scope *scope);

        std::unique_ptr<Value> generateControl(MaximAst::ControlExpression *expr, Function *function, Scope *scope);

        std::unique_ptr<Value> generateVariable(MaximAst::VariableExpression *expr, Function *function, Scope *scope);

        // operators
        std::unique_ptr<Value> generateMath(MaximAst::MathExpression *expr, Function *function, Scope *scope);

        llvm::Value *generateFloatIntCompMath(MaximAst::MathExpression::Type type, llvm::Value *leftVal,
                                              llvm::Value *rightVal, Function *function);

        llvm::Value *generateIntCompMath(MaximAst::MathExpression::Type type, llvm::Value *leftVal,
                                         llvm::Value *rightVal, Function *function);

        llvm::Value *generateCompareMath(MaximAst::MathExpression::Type type, llvm::Value *leftVal,
                                         llvm::Value *rightVal, Function *function);

        std::unique_ptr<Value> generateUnary(MaximAst::UnaryExpression *expr, Function *function, Scope *scope);

        // assignments
        std::unique_ptr<Value> generateAssign(MaximAst::AssignExpression *expr, Function *function, Scope *scope);

        void generateSingleAssign(MaximAst::AssignableExpression *leftExpr, Value *rightValue,
                                  MaximAst::AssignExpression::Type type, SourcePos rightStart, SourcePos rightEnd,
                                  Function *function, Scope *scope);

        void generateBasicAssign(MaximAst::AssignableExpression *leftExpr, Value *rightValue, Function *function,
                                 Scope *scope);

        void generateVariableAssign(MaximAst::VariableExpression *leftExpr, Value *rightValue, Function *function,
                                    Scope *scope);

        void generateControlAssign(MaximAst::ControlExpression *leftExpr, Value *rightValue, Function *function,
                                   Scope *scope);

        std::unique_ptr<Value> generatePostfix(MaximAst::PostfixExpression *expr, Function *function, Scope *scope);

        std::unique_ptr<NumValue> evaluateConstVal(std::unique_ptr<NumValue> value);

        std::unique_ptr<MidiValue> evaluateConstVal(std::unique_ptr<MidiValue> value);

        std::unique_ptr<TupleValue> evaluateConstVal(std::unique_ptr<TupleValue> value);
    };

}
