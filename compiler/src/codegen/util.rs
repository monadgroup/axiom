use codegen::intrinsics;
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicTypeEnum, FunctionType, VectorType};
use inkwell::values::VectorValue;
use inkwell::values::{FunctionValue, IntValue, PointerValue};

pub fn get_size_of(t: &BasicTypeEnum) -> Option<IntValue> {
    match t {
        BasicTypeEnum::IntType(val) => Some(val.size_of()),
        BasicTypeEnum::FloatType(val) => Some(val.size_of()),
        BasicTypeEnum::PointerType(val) => Some(val.size_of()),
        BasicTypeEnum::StructType(val) => val.size_of(),
        BasicTypeEnum::ArrayType(val) => val.size_of(),
        BasicTypeEnum::VectorType(val) => val.size_of(),
    }
}

pub fn get_or_create_func(
    module: &Module,
    name: &str,
    func_type: &FunctionType,
    linkage: Option<&Linkage>,
) -> FunctionValue {
    if let Some(func) = module.get_function(name) {
        func
    } else {
        module.add_function(name, func_type, linkage)
    }
}

pub fn get_const_vec(context: &Context, left: f32, right: f32) -> VectorValue {
    VectorType::const_vector(&[
        &context.f32_type().const_float(left as f64),
        &context.f32_type().const_float(right as f64),
    ])
}

pub fn get_vec_spread(context: &Context, val: f32) -> VectorValue {
    get_const_vec(context, val, val)
}

pub fn copy_ptr(builder: &mut Builder, module: &Module, src: &PointerValue, dest: &PointerValue) {
    let src_elem_type = src.get_type().element_type();
    let dest_elem_type = dest.get_type().element_type();
    assert_eq!(src_elem_type, dest_elem_type);

    let param_size = get_size_of(&src_elem_type).unwrap();
    let context = module.get_context();
    builder.build_call(
        &intrinsics::memcpy(module),
        &[
            dest,
            src,
            &param_size,
            &context.i32_type().const_int(0, false),
            &context.bool_type().const_int(0, false),
        ],
        "",
        false,
    );
}
