#include <iostream>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

#include "parser/TokenStream.h"
#include "parser/Parser.h"
#include "ast/Expression.h"
#include "ast/Block.h"

#include "codegen/MaximContext.h"
#include "codegen/visitors/ExpressionVisitor.h"
#include "codegen/ComposableModuleClass.h"
#include "codegen/ComposableModuleClassMethod.h"
#include "codegen/Scope.h"
#include "codegen/Value.h"
#include "codegen/Operator.h"
#include "codegen/Function.h"
#include "codegen/Control.h"

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

void parseAndCompile(MaximContext *ctx, llvm::Module *mainModule) {
    auto input = getInput();
    auto stream = std::make_unique<TokenStream>(input);
    Parser parser(std::move(stream));
    auto block = parser.parse();

    llvm::Module nodeModule("node", ctx->llvm());
    ComposableModuleClass node(ctx, &nodeModule, "node");
    ComposableModuleClassMethod generateFunc(&node, "generate");
    Scope scope;

    for (const auto &expr : block->expressions) {
        visitExpression(&generateFunc, &scope, expr.get());
    }
    generateFunc.builder().CreateRetVoid();
    node.complete();

    llvm::Linker::linkModules(nodeModule, llvm::CloneModule(*mainModule));
    nodeModule.print(llvm::errs(), nullptr);
}

int main() {
    llvm::InitializeNativeTarget();

    //llvm::Module mainModule("main")
    MaximContext ctx(llvm::EngineBuilder().selectTarget()->createDataLayout());
    llvm::Module mainModule("main", ctx.llvm());
    ctx.setLibModule(&mainModule);

    while (true) {
        try {
            parseAndCompile(&ctx, &mainModule);
        } catch (const MaximCommon::CompileError &err) {
            std::cout << "Error from " << err.start.line << ":" << err.start.column << " to " << err.end.line
                      << ":" << err.end.column << std::endl;
            std::cout << err.message << std::endl;
            return 0;
        }
    }
}
