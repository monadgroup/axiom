#include <iostream>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include "parser/TokenStream.h"
#include "parser/Parser.h"
#include "ast/Expression.h"
#include "ast/Block.h"

#include "codegen/MaximContext.h"
#include "codegen/Node.h"
#include "codegen/visitors/ExpressionVisitor.h"
#include "codegen/Value.h"
#include "codegen/Operator.h"
#include "codegen/Converter.h"
#include "codegen/Function.h"
#include "codegen/Num.h"

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
    Node node(ctx, &nodeModule);

    auto numType = ctx->numType();
    auto global = new llvm::GlobalVariable(
        nodeModule, numType->get(), false, llvm::GlobalValue::LinkageTypes::ExternalLinkage,
        llvm::ConstantStruct::get(numType->get(), {
            llvm::ConstantVector::get({ctx->constFloat(0), ctx->constFloat(0)}),
            llvm::ConstantInt::get(numType->formType(), (uint64_t) MaximCommon::FormType::LINEAR, false),
            llvm::ConstantInt::get(numType->activeType(), (uint64_t) true, false)
        }), "result"
    );

    std::unique_ptr<Value> lastVal;
    for (const auto &expr : block->expressions) {
        lastVal = visitExpression(&node, expr.get());
    }
    if (lastVal) {
        if (auto numVal = dynamic_cast<Num *>(lastVal.get())) {
            node.builder().CreateStore(numVal->get(), global);
        }
    }

    node.builder().CreateRetVoid();
    node.complete();

    auto mainFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm()), {}),
        llvm::Function::LinkageTypes::ExternalLinkage, "main", &nodeModule
    );
    auto funcBlock = llvm::BasicBlock::Create(ctx->llvm(), "entry", mainFunc);
    Builder mainBuilder(funcBlock);
    auto ctxType = node.type(ctx);
    auto ctxConst = node.getInitialVal(ctx);
    auto ctxGlobal = new llvm::GlobalVariable(nodeModule, ctxType, false,
                                              llvm::GlobalValue::LinkageTypes::InternalLinkage, ctxConst, "ctx");
    node.initializeVal(ctx, &nodeModule, ctxGlobal, mainBuilder);
    mainBuilder.CreateRetVoid();

    llvm::verifyFunction(*node.func());
    llvm::verifyFunction(*mainFunc);

    llvm::Linker::linkModules(nodeModule, llvm::CloneModule(mainModule));
    nodeModule.print(llvm::errs(), nullptr);
}

int main() {
    MaximContext ctx;
    llvm::Module mainModule("main", ctx.llvm());
    ctx.buildFunctions(&mainModule);

    while (true) {
        try {
            parseAndCompile(&ctx, &mainModule);
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
