#include "Converter.h"

#include "MaximContext.h"
#include "Num.h"
#include "Node.h"

using namespace MaximCodegen;

Converter::Converter(MaximContext *context, MaximCommon::FormType toType) : _context(context), _toType(toType) {

}

void Converter::generate(llvm::Module *module) {
    auto func = createFuncForModule(module);
    auto undefPos = SourcePos(-1, -1);
    auto numInput = Num::create(context(), func->arg_begin(), undefPos, undefPos);

    auto entryBlock = llvm::BasicBlock::Create(context()->llvm(), "entry", func);
    Builder b(entryBlock);
    auto numForm = numInput->form(b);
    auto numVec = numInput->vec(b);

    auto defaultBlock = llvm::BasicBlock::Create(context()->llvm(), "default", func);
    Builder defaultB(defaultBlock);
    auto defaultNum = numInput->withForm(b, _toType, undefPos, undefPos);
    defaultB.CreateRet(defaultNum->get());

    auto formSwitch = b.CreateSwitch(numForm, defaultBlock, (unsigned int) converters.size());
    for (const auto &pair : converters) {
        auto blockName = "branch." + MaximCommon::formType2String(pair.first);
        auto converterBlock = llvm::BasicBlock::Create(context()->llvm(), blockName, func);
        formSwitch->addCase(
            llvm::ConstantInt::get(context()->numType()->formType(), (uint64_t) pair.first, false),
            converterBlock
        );
        b.SetInsertPoint(converterBlock);

        auto convertFunc = pair.second;
        auto newVec = (this->*convertFunc)(b, numVec, module);
        auto newNum = numInput->withVec(b, newVec, undefPos, undefPos)->withForm(b, _toType, undefPos, undefPos);
        b.CreateRet(newNum->get());
    }
}

std::unique_ptr<Num> Converter::call(Node *node, std::unique_ptr<Num> value, SourcePos startPos, SourcePos endPos) {
    // if the input is constant, constant-fold the converter
    auto realVal = value->get();
    if (auto constVal = llvm::dyn_cast<llvm::Constant>(realVal)) {
        auto formVal = llvm::cast<llvm::ConstantInt>(constVal->getAggregateElement(1))->getZExtValue();
        auto formConverter = converters.find((MaximCommon::FormType) formVal);

        if (formConverter == converters.end()) {
            return value->withForm(node->builder(), _toType, startPos, endPos);
        } else {
            auto vecVal = constVal->getAggregateElement((unsigned int) 0);
            auto converterFunc = formConverter->second;
            auto newVec = (this->*converterFunc)(node->builder(), vecVal, node->module());
            return value->withVec(node->builder(), newVec, startPos, endPos)->withForm(node->builder(), _toType, startPos, endPos);
        }
    } else {
        // otherwise call the generated function
        auto result = CreateCall(node->builder(), createFuncForModule(node->module()), {realVal}, "result");
        return Num::create(context(), result, startPos, endPos);
    }
}

llvm::Function* Converter::createFuncForModule(llvm::Module *module) {
    auto funcType = llvm::FunctionType::get(context()->numType()->get(), {context()->numType()->get()}, false);
    auto funcName = "maximconverter." + MaximCommon::formType2String(toType());
    return llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, module);
}
