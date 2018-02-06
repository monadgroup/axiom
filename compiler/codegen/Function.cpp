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
    paramValues.reserve(params.size());

    size_t consumedParams = 0;

    // handle all required + optional parameters
    for (size_t i = 0; i < _decl->parameters().size(); i++) {
        if (i >= params.size()) break;

        auto paramData = &_decl->parameters()[i];
        auto paramItem = &params[i];
        consumedParams++;
        checkParam(paramItem, paramData);

        resultConst = resultConst && paramItem->value->isConst();
        paramValues.push_back(function->codeBuilder().CreateLoad(paramItem->value->value(), "call_param_temp"));
    }

    // handle all remaining optional parameters
    for (size_t i = paramValues.size(); i < _decl->parameters().size(); i++) {
        auto paramData = &_decl->parameters()[i];
        assert(paramData->defaultValue());
        paramValues.push_back(paramData->defaultValue());
    }

    // handle varargs
    if (_decl->variadicParam()) {
        auto vaData = _decl->variadicParam();
        auto remainingParams = params.size() - consumedParams;
        auto countValue = llvm::ConstantInt::get(llvm::Type::getInt8Ty(_context->llvm()), remainingParams, false);
        assert(remainingParams > 0);

        auto vaStruct = function->initBuilder().CreateAlloca(_decl->vaType(), nullptr, "va_args");
        function->codeBuilder().CreateStore(
                countValue,
                _context->getPtr(vaStruct, 0, function->codeBuilder())
        );

        auto vaArray = function->initBuilder().CreateAlloca(
                vaData->type(),
                countValue,
                "va_arg_arr"
        );
        for (auto i = consumedParams; i < params.size(); i++) {
            auto paramItem = &params[i];
            checkParam(paramItem, vaData);

            resultConst = resultConst && paramItem->value->isConst();
            auto storePos = function->codeBuilder().Insert(
                    llvm::GetElementPtrInst::Create(vaData->type(), vaArray, {
                            _context->getConstantInt(32, i, false)
                    }),
                    "va_store_pos"
            );
            function->codeBuilder().CreateStore(
                    function->codeBuilder().CreateLoad(paramItem->value->value(), "call_param_temp"),
                    storePos
            );
        }
        function->codeBuilder().CreateStore(
                vaArray,
                _context->getPtr(vaStruct, 1, function->codeBuilder())
        );
        paramValues.push_back(function->codeBuilder().CreateLoad(vaStruct, "va_args_temp"));
    }

    auto returnDest = function->initBuilder().CreateAlloca(_decl->returnType(), nullptr, "call_result");
    auto returnVal = function->codeBuilder().CreateCall(_llFunc, paramValues, "call_result_temp");
    function->codeBuilder().CreateStore(returnVal, returnDest);
    return _context->llToValue(resultConst, returnDest);
}

void Function::checkParam(const ParamData *paramItem, const Parameter *paramData) {
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
}
