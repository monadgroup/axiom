#include "Function.h"

#include "Context.h"
#include "values/Value.h"
#include "Control.h"
#include "CodegenError.h"

using namespace MaximCodegen;

Function::Function(std::unique_ptr<FunctionDeclaration> decl, const std::string &name, llvm::Function::LinkageTypes linkage, llvm::Module *module, Context *context)
        : _context(context), _scope(_context), _decl(std::move(decl)), _initBuilder(context->llvm()),
          _codeBuilder(context->llvm()) {
    _llFunc = llvm::Function::Create(_decl->type(), linkage, name, module);

    _initBlock = llvm::BasicBlock::Create(context->llvm(), "entry", _llFunc);
    _initBuilder.SetInsertPoint(_initBlock);

    _codeBlock = llvm::BasicBlock::Create(context->llvm(), "code", _llFunc);
    _codeBuilder.SetInsertPoint(_codeBlock);
}

std::unique_ptr<Value> Function::generateCall(const std::vector<ParamData> &params, SourcePos start, SourcePos end, Function *function) {
    if (params.size() < _decl->minParamCount()) {
        throw CodegenError("Eyy! I need more parameters than that my dude.", start, end);
    }

    auto maxParamCount = _decl->maxParamCount();
    if (params.size() > maxParamCount && maxParamCount >= 0) {
        throw CodegenError("Eyy! I need less parameters than that my dude.", start, end);
    }

    auto resultConst = _decl->isPure();
    std::vector<llvm::Value *> paramValues;
    paramValues.reserve(params.size() + 1);
    paramValues.push_back(nullptr);

    // handle all parameters passed in
    for (size_t i = 0; i < params.size(); i++) {
        auto paramData = _decl->getParameter(i);
        auto paramItem = &params[i];
        _context->checkPtrType(
                paramItem->value->value(),
                _context->getType(paramData->type()),
                paramItem->start,
                paramItem->end
        );

        if (paramData->isConst() && !paramItem->value->isConst()) {
            throw CodegenError(
                    "I constantly insist that constant values must be passed into constant parameters, and yet they consistently aren't constant.",
                    paramItem->start, paramItem->end
            );
        }
        if (!paramItem->value->isConst()) resultConst = false;

        paramValues.push_back(function->codeBuilder().CreateLoad(paramItem->value->value(), "call_param_temp"));
    }

    // add in extra parameters
    for (size_t i = 0; i < _decl->parameters().size(); i++) {
        auto param = _decl->parameters()[i];
        if (param.defaultValue() && paramValues.size() <= i) {
            paramValues.push_back(param.defaultValue());
        }
    }

    // add number of arguments passed
    paramValues[0] = llvm::ConstantInt::get(llvm::Type::getInt8Ty(_context->llvm()), paramValues.size() - 1);

    auto returnDest = function->initBuilder().CreateAlloca(_decl->returnType(), nullptr, "call_result");
    auto returnVal = function->codeBuilder().CreateCall(_llFunc, paramValues, "call_result_temp");
    function->codeBuilder().CreateStore(returnVal, returnDest);
    return _context->llToValue(resultConst, returnDest);
}
