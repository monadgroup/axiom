use codegen::util;
use inkwell::module::{Linkage, Module};
use inkwell::types::BasicType;
use inkwell::values::FunctionValue;
use inkwell::AddressSpace;

pub fn memcpy(module: &Module) -> FunctionValue {
    // todo: set correct attributes on arguments
    // todo: use 64 or 32-bits depending on data layout
    util::get_or_create_func(module, "llvm.memcpy.p0i8.p0i8.i64", &|| {
        let context = module.get_context();
        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(
                &[
                    &context.i8_type().ptr_type(AddressSpace::Generic),
                    &context.i8_type().ptr_type(AddressSpace::Generic),
                    &context.i64_type(),
                    &context.i32_type(),
                    &context.bool_type(),
                ],
                false,
            ),
        )
    })
}

pub fn pow_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.pow.v2f32", &|| {
        let context = module.get_context();
        let v2f32_type = context.f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type, &v2f32_type], false),
        )
    })
}

pub fn log_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.log.v2f32", &|| {
        let context = module.get_context();
        let v2f32_type = context.f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn log10_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.log10.v2f32", &|| {
        let context = module.get_context();
        let v2f32_type = context.f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn log2_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.log2.v2f32", &|| {
        let context = module.get_context();
        let v2f32_type = context.f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn cos_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.cos.v2f32", &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn sin_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.sin.v2f32", &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn sqrt_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.sqrt.v2f32", &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn ceil_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.ceil.v2f32", &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn floor_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.floor.v2f32", &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn fabs_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.fabs.v2f32", &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}
