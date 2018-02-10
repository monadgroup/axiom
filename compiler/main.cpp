#include <iostream>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>

#include "parser/TokenStream.h"
#include "parser/Parser.h"
#include "ast/Expression.h"
#include "ast/Block.h"

#include "codegen/Context.h"
#include "codegen/Function.h"
#include "codegen/ExpressionGenerator.h"
#include "codegen/FunctionDeclaration.h"
#include "codegen/ControlDeclaration.h"
#include "codegen/CodegenError.h"
#include "codegen/values/Value.h"
#include "codegen/values/NumValue.h"

#include "util.h"

using namespace MaximParser;
using namespace MaximCodegen;

std::string getInput() {
    std::cin >> std::noskipws;

    std::string accumulate;
    std::string lastLine;

    while (std::getline(std::cin, lastLine)) {
        if (lastLine == "end") break;
        accumulate += lastLine + "\n";
    }
    return accumulate;
}

void parseAndCompile() {
    auto input = getInput();
    auto stream = std::make_unique<TokenStream>(input);
    Parser parser(std::move(stream));
    auto block = parser.parse();

    Context context;

    auto vecType = llvm::VectorType::get(llvm::Type::getFloatTy(context.llvm()), 2);
    auto mainFuncDecl = std::make_unique<FunctionDeclaration>(false, vecType, std::vector<Parameter>{});
    auto fun = Function(std::move(mainFuncDecl), "main", llvm::Function::ExternalLinkage, context.builtinModule(),
                        &context);
    auto exprGen = ExpressionGenerator(&context);

    MaximAst::Expression *lastExpr = nullptr;
    std::unique_ptr<Value> lastVal;
    for (const auto &expr : block->expressions) {
        lastExpr = expr.get();
        lastVal = exprGen.generateExpr(lastExpr, &fun, fun.scope());
    }
    if (lastVal && lastExpr) {
        context.checkPtrType(lastVal->value(), Context::Type::NUM, lastExpr->startPos, lastExpr->endPos);
        auto numValue = AxiomUtil::strict_unique_cast<NumValue>(std::move(lastVal));

        fun.codeBuilder().CreateRet(fun.codeBuilder().CreateLoad(numValue->valuePtr(fun.codeBuilder()), "ret_load"));
    }

    fun.initBuilder().CreateBr(fun.codeBlock());

    llvm::verifyFunction(*fun.llFunc());

    context.builtinModule()->print(llvm::errs(), nullptr);
}

int main() {
    while (true) {
        try {
            parseAndCompile();
        } catch (const ParseError &err) {
            std::cout << "Parse error from " << err.start.line << ":" << err.start.column << " to " << err.end.line
                      << ":" << err.end.column << std::endl;
            std::cout << err.message << std::endl;
        } catch (const CodegenError &err) {
            std::cout << "Compile error from " << err.start.line << ":" << err.start.column << " to " << err.end.line
                      << ":" << err.end.column << std::endl;
            std::cout << err.message << std::endl;
        }
    }
}
