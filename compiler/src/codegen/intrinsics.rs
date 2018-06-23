use codegen::util;
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicType, BasicTypeEnum};
use inkwell::values::FunctionValue;
use inkwell::AddressSpace;

pub fn memcpy(module: &Module) -> FunctionValue {
    // todo: set correct attributes on arguments
    // todo: use 64 or 32-bits depending on data layout
    let context = module.get_context();
    util::get_or_create_func(
        module,
        "llvm.memcpy.p0i8.p0i8.i64",
        &context.void_type().fn_type(
            &[
                &BasicTypeEnum::from(context.i8_type().ptr_type(AddressSpace::Generic)),
                &BasicTypeEnum::from(context.i8_type().ptr_type(AddressSpace::Generic)),
                &BasicTypeEnum::from(context.i64_type()),
                &BasicTypeEnum::from(context.i32_type()),
                &BasicTypeEnum::from(context.bool_type()),
            ],
            false,
        ),
        Some(&Linkage::ExternalLinkage),
    )
}

pub fn pow_v2f32(module: &Module) -> FunctionValue {
    let context = module.get_context();
    let v2f32_type = context.f32_type().vec_type(2);

    util::get_or_create_func(
        module,
        "llvm.pow.v2f32",
        &v2f32_type.fn_type(
            &[
                &BasicTypeEnum::from(v2f32_type),
                &BasicTypeEnum::from(v2f32_type),
            ],
            false,
        ),
        Some(&Linkage::ExternalLinkage),
    )
}

pub fn log_v2f32(module: &Module) -> FunctionValue {
    let context = module.get_context();
    let v2f32_type = context.f32_type().vec_type(2);
    util::get_or_create_func(
        module,
        "llvm.log.v2f32",
        &v2f32_type.fn_type(&[&BasicTypeEnum::from(v2f32_type)], false),
        Some(&Linkage::ExternalLinkage),
    )
}

pub fn log10_v2f32(module: &Module) -> FunctionValue {
    let context = module.get_context();
    let v2f32_type = context.f32_type().vec_type(2);
    util::get_or_create_func(
        module,
        "llvm.log10.v2f32",
        &v2f32_type.fn_type(&[&BasicTypeEnum::from(v2f32_type)], false),
        Some(&Linkage::ExternalLinkage),
    )
}

pub fn log2_v2f32(module: &Module) -> FunctionValue {
    let context = module.get_context();
    let v2f32_type = context.f32_type().vec_type(2);
    util::get_or_create_func(
        module,
        "llvm.log2.v2f32",
        &v2f32_type.fn_type(&[&BasicTypeEnum::from(v2f32_type)], false),
        Some(&Linkage::ExternalLinkage),
    )
}
