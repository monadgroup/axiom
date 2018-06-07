#include "Exporter.h"

#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include "../codegen/MaximContext.h"
#include "../codegen/Midi.h"
#include "Runtime.h"

using namespace MaximRuntime;

Exporter::Exporter(MaximCodegen::MaximContext *context, const llvm::Module *commonModule)
    : module("export", context->llvm()) {
    target = llvm::EngineBuilder().selectTarget();
    module.setTargetTriple(target->getTargetTriple().str());
    module.setDataLayout(target->createDataLayout());

    target->Options.FPDenormalMode = llvm::FPDenormal::PositiveZero;
    target->Options.NoSignedZerosFPMath = 1;
    target->Options.AllowFPOpFusion = llvm::FPOpFusion::Fast;
    target->Options.NoTrappingFPMath = 1;
    target->Options.UnsafeFPMath = 1;
    target->setOptLevel(llvm::CodeGenOpt::Aggressive);

    llvm::Linker linker(module);
    linker.linkInModule(llvm::CloneModule(commonModule));

    _exportDefinitionTy = llvm::StructType::get(context->llvm(), {
        context->dataLayoutType(), // storage alloc size
        llvm::PointerType::get(llvm::FunctionType::get(llvm::Type::getVoidTy(context->llvm()), {context->voidPointerType()}, false), 0), // constructor func
        llvm::PointerType::get(llvm::FunctionType::get(llvm::Type::getVoidTy(context->llvm()), {context->voidPointerType()}, false), 0), // generate func
        llvm::PointerType::get(llvm::FunctionType::get(llvm::Type::getVoidTy(context->llvm()), {context->voidPointerType()}, false), 0)  // destructor func
    }, false);

    _exportInstrumentTy = llvm::StructType::get(context->llvm(), {
        context->voidPointerType(),
        llvm::PointerType::get(_exportDefinitionTy, 0)
    }, false);

    buildInterfaceFunctions(context);
}

void Exporter::addRuntime(MaximRuntime::Runtime *runtime, const std::string &exportName) {
    llvm::Linker linker(module);
    runtime->jit().linker = &linker;
    runtime->mainSurface()->scheduleChildUpdate();
    auto mainClass = runtime->compile();
    runtime->jit().linker = nullptr;

    auto runConstructor = buildInstrumentFunc(runtime->ctx(), exportName + ".constructor", mainClass->constructor());
    auto runGenerate = buildInstrumentFunc(runtime->ctx(), exportName + ".generate", mainClass->generate());
    auto runDestructor = buildInstrumentFunc(runtime->ctx(), exportName + ".destructor", mainClass->generate());

    auto exportGlobal = new llvm::GlobalVariable(
        module, _exportDefinitionTy, false, llvm::GlobalValue::LinkageTypes::ExternalLinkage,
        llvm::ConstantStruct::get(_exportDefinitionTy, {
            runtime->ctx()->sizeOf(mainClass->storageType()),
            runConstructor,
            runGenerate,
            runDestructor
        }), exportName
    );
    exportGlobal->setConstant(true);
}

void Exporter::exportObject(llvm::raw_fd_ostream &dest, unsigned optLevel, unsigned sizeLevel) {
    finishModule(optLevel, sizeLevel);

    llvm::legacy::PassManager pass;
    auto addPassSuccess = !target->addPassesToEmitFile(pass, dest, llvm::TargetMachine::CGFT_ObjectFile);
    assert(addPassSuccess);
    pass.run(module);
    dest.flush();
}

void Exporter::exportLto(llvm::raw_fd_ostream &dest, unsigned optLevel, unsigned sizeLevel) {
    finishModule(optLevel, sizeLevel);

    llvm::WriteBitcodeToFile(&module, dest);
    dest.flush();
}

void Exporter::finishModule(unsigned optLevel, unsigned sizeLevel) {
    // ensure all maxim.* globals in the module are private
    for (auto &global : module.global_values()) {
        if (global.getName().startswith("maxim")) {
            global.setLinkage(llvm::GlobalValue::PrivateLinkage);
        }
    }

    // run the optimizer
    Jit::optimizeModule(&module, target, optLevel, sizeLevel);

    module.print(llvm::errs(), nullptr);
}

void Exporter::buildInterfaceFunctions(MaximCodegen::MaximContext *ctx) {
    buildCreateInstrumentFunc(ctx);
    buildGetInputFunc(ctx);
    buildGetOutputFunc(ctx);
    buildGenerateFunc(ctx);
    buildDestroyInstrumentFunc(ctx);
    buildMidiPushFunc(ctx);
    buildMidiClearFunc(ctx);
    buildNumWriteFunc(ctx);
    buildNumReadFunc(ctx);
}

void Exporter::buildCreateInstrumentFunc(MaximCodegen::MaximContext *ctx) {
    auto allocFunction = module.getFunction("malloc");
    if (!allocFunction) {
        allocFunction = llvm::Function::Create(
            llvm::FunctionType::get(ctx->voidPointerType(), {ctx->dataLayoutType()}, false),
            llvm::Function::ExternalLinkage, "malloc", &module
        );
    }
    auto memSetFunc = llvm::Intrinsic::getDeclaration(&module, llvm::Intrinsic::memset, {llvm::IntegerType::getInt8PtrTy(ctx->llvm()), ctx->dataLayoutType()});

    auto createInstrumentFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::PointerType::get(_exportInstrumentTy, 0), {llvm::PointerType::get(_exportDefinitionTy, 0)}, false),
        llvm::Function::LinkageTypes::ExternalLinkage, "axiom_create_instrument", &module
    );
    auto block = llvm::BasicBlock::Create(ctx->llvm(), "entry", createInstrumentFunc);
    MaximCodegen::Builder b(block);

    auto allocSize = b.CreateLoad(b.CreateStructGEP(_exportDefinitionTy, createInstrumentFunc->arg_begin(), 0));

    // for simplicity, we can put the allocated buffer as the last element in the structure
    auto instrumentSize = ctx->sizeOf(_exportInstrumentTy);
    auto resultSize = b.CreateAdd(allocSize, instrumentSize);

    auto resultPtr = b.CreatePointerCast(b.CreateCall(allocFunction, {resultSize}), llvm::PointerType::get(_exportInstrumentTy, 0));

    auto dataPtr = b.CreateGEP(b.CreatePointerCast(resultPtr, llvm::IntegerType::getInt8PtrTy(ctx->llvm())), instrumentSize);

    b.CreateStore(b.CreatePointerCast(dataPtr, ctx->voidPointerType()), b.CreateStructGEP(_exportInstrumentTy, resultPtr, 0));
    b.CreateStore(createInstrumentFunc->arg_begin(), b.CreateStructGEP(_exportInstrumentTy, resultPtr, 1));

    // fill the allocated buffer with zeros
    b.CreateCall(memSetFunc, {dataPtr, ctx->constInt(8, 0, false), allocSize, ctx->constInt(32, 0, false), ctx->constInt(1, 0, false)});

    // run the constructor function to initialize everything
    auto constructorFunc = b.CreateLoad(b.CreateStructGEP(_exportDefinitionTy, createInstrumentFunc->arg_begin(), 1));
    b.CreateCall(constructorFunc, {b.CreatePointerCast(dataPtr, ctx->voidPointerType())});
    b.CreateRet(resultPtr);
}

void Exporter::buildGetInputFunc(MaximCodegen::MaximContext *ctx) {
    // todo
}

void Exporter::buildGetOutputFunc(MaximCodegen::MaximContext *ctx) {
    // todo
}

void Exporter::buildGenerateFunc(MaximCodegen::MaximContext *ctx) {
    auto generateInstrumentFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm()), {llvm::PointerType::get(_exportInstrumentTy, 0)}, false),
        llvm::Function::LinkageTypes::ExternalLinkage, "axiom_generate", &module
    );
    auto block = llvm::BasicBlock::Create(ctx->llvm(), "entry", generateInstrumentFunc);
    MaximCodegen::Builder b(block);

    auto dataPtr = b.CreateLoad(b.CreateStructGEP(_exportInstrumentTy, generateInstrumentFunc->arg_begin(), 0));
    auto definition = b.CreateLoad(b.CreateStructGEP(_exportInstrumentTy, generateInstrumentFunc->arg_begin(), 1));
    auto generateFunc = b.CreateStructGEP(_exportDefinitionTy, definition, 2);
    b.CreateCall(b.CreateLoad(generateFunc), {dataPtr});
    b.CreateRetVoid();
}

void Exporter::buildDestroyInstrumentFunc(MaximCodegen::MaximContext *ctx) {
    auto freeFunction = module.getFunction("free");
    if (!freeFunction) {
        freeFunction = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm()), {llvm::Type::getInt8PtrTy(ctx->llvm())}, false),
            llvm::Function::ExternalLinkage, "free", &module
        );
    }

    auto destroyInstrumentFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm()), {llvm::PointerType::get(_exportInstrumentTy, 0)}, false),
        llvm::Function::LinkageTypes::ExternalLinkage, "axiom_destroy_instrument", &module
    );
    auto block = llvm::BasicBlock::Create(ctx->llvm(), "entry", destroyInstrumentFunc);
    MaximCodegen::Builder b(block);

    b.CreateCall(freeFunction, {b.CreatePointerCast(destroyInstrumentFunc->arg_begin(), llvm::Type::getInt8PtrTy(ctx->llvm()))});
    b.CreateRetVoid();
}

void Exporter::buildMidiPushFunc(MaximCodegen::MaximContext *ctx) {
    auto midiPushFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm()), {llvm::PointerType::get(ctx->midiType()->get(), 0), ctx->midiType()->eventType()}, false),
        llvm::Function::LinkageTypes::ExternalLinkage, "axiom_midi_push", &module
    );
    auto block = llvm::BasicBlock::Create(ctx->llvm(), "entry", midiPushFunc);
    MaximCodegen::Builder b(block);

    // the event value is expected to be passed as a pointer, so we need to store it in an alloc
    auto eventPtr = b.CreateAlloca(ctx->midiType()->eventType());
    b.CreateStore(midiPushFunc->arg_begin() + 1, eventPtr);

    auto midiInput = MaximCodegen::Midi::create(ctx, midiPushFunc->arg_begin(), SourcePos(-1, -1), SourcePos(-1, -1));
    MaximCodegen::MidiEvent event(eventPtr, ctx->midiType()->eventType());
    midiInput->pushEvent(b, event, &module);
    b.CreateRetVoid();
}

void Exporter::buildMidiClearFunc(MaximCodegen::MaximContext *ctx) {
    auto midiClearFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm()), {llvm::PointerType::get(ctx->midiType()->get(), 0)}, false),
        llvm::Function::LinkageTypes::ExternalLinkage, "axiom_midi_clear", &module
    );
    auto block = llvm::BasicBlock::Create(ctx->llvm(), "entry", midiClearFunc);
    MaximCodegen::Builder b(block);

    auto midiInput = MaximCodegen::Midi::create(ctx, midiClearFunc->arg_begin(), SourcePos(-1, -1), SourcePos(-1, -1));
    midiInput->setCount(b, (uint64_t) 0);
    b.CreateRetVoid();
}

void Exporter::buildNumWriteFunc(MaximCodegen::MaximContext *ctx) {
    auto numWriteFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm()), {llvm::PointerType::get(ctx->numType()->get(), 0), ctx->numType()->get()}, false),
        llvm::Function::LinkageTypes::ExternalLinkage, "axiom_num_write", &module
    );
    auto block = llvm::BasicBlock::Create(ctx->llvm(), "entry", numWriteFunc);
    MaximCodegen::Builder b(block);

    b.CreateStore(numWriteFunc->arg_begin() + 1, numWriteFunc->arg_begin());
    b.CreateRetVoid();
}

void Exporter::buildNumReadFunc(MaximCodegen::MaximContext *ctx) {
    auto numReadFunc = llvm::Function::Create(
        llvm::FunctionType::get(ctx->numType()->get(), {llvm::PointerType::get(ctx->numType()->get(), 0)}, false),
        llvm::Function::LinkageTypes::ExternalLinkage, "axiom_num_read", &module
    );
    auto block = llvm::BasicBlock::Create(ctx->llvm(), "entry", numReadFunc);
    MaximCodegen::Builder b(block);

    b.CreateRet(b.CreateLoad(numReadFunc->arg_begin()));
}

llvm::Function* Exporter::buildInstrumentFunc(MaximCodegen::MaximContext *ctx, const std::string &name,
                                              MaximCodegen::ModuleClassMethod *method) {
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm()), {ctx->voidPointerType()}, false),
        llvm::Function::LinkageTypes::PrivateLinkage, name, &module
    );
    auto block = llvm::BasicBlock::Create(ctx->llvm(), "entry", func);
    MaximCodegen::Builder b(block);
    method->call(b, {}, func->arg_begin(), &module, "");
    b.CreateRetVoid();
    return func;
}
