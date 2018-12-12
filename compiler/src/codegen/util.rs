use crate::codegen::intrinsics;
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::{ArrayType, BasicType, BasicTypeEnum, FunctionType, VectorType};
use inkwell::values::{
    ArrayValue, BasicValueEnum, FloatValue, FunctionValue, GlobalValue, IntValue, PointerValue,
    VectorValue,
};
use inkwell::AddressSpace;
use inkwell::IntPredicate;

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

pub fn get_array_type(t: &BasicTypeEnum, size: u32) -> ArrayType {
    match t {
        BasicTypeEnum::IntType(val) => val.array_type(size),
        BasicTypeEnum::FloatType(val) => val.array_type(size),
        BasicTypeEnum::PointerType(val) => val.array_type(size),
        BasicTypeEnum::StructType(val) => val.array_type(size),
        BasicTypeEnum::ArrayType(val) => val.array_type(size),
        BasicTypeEnum::VectorType(val) => val.array_type(size),
    }
}

pub fn get_constant_array(t: &BasicTypeEnum, items: &[BasicValueEnum]) -> ArrayValue {
    match t {
        BasicTypeEnum::IntType(ty) => ty.const_array(
            &items
                .iter()
                .map(|val| val.into_int_value())
                .collect::<Vec<_>>(),
        ),
        BasicTypeEnum::FloatType(ty) => ty.const_array(
            &items
                .iter()
                .map(|val| val.into_float_value())
                .collect::<Vec<_>>(),
        ),
        BasicTypeEnum::PointerType(ty) => ty.const_array(
            &items
                .iter()
                .map(|val| val.into_pointer_value())
                .collect::<Vec<_>>(),
        ),
        BasicTypeEnum::StructType(ty) => ty.const_array(
            &items
                .iter()
                .map(|val| val.into_struct_value())
                .collect::<Vec<_>>(),
        ),
        BasicTypeEnum::ArrayType(ty) => ty.const_array(
            &items
                .iter()
                .map(|val| val.into_array_value())
                .collect::<Vec<_>>(),
        ),
        BasicTypeEnum::VectorType(ty) => ty.const_array(
            &items
                .iter()
                .map(|val| val.into_vector_value())
                .collect::<Vec<_>>(),
        ),
    }
}

pub fn get_or_create_func(
    module: &Module,
    name: &str,
    is_internal: bool,
    cb: &Fn() -> (Linkage, FunctionType),
) -> FunctionValue {
    if let Some(func) = module.get_function(name) {
        func
    } else {
        let (linkage, func_type) = cb();
        let func = module.add_function(name, &func_type, Some(&linkage));

        if is_internal {
            // fastcc is 8 according to http://llvm.org/doxygen/namespacellvm_1_1CallingConv.html
            func.set_call_conventions(8);
        }

        func
    }
}

pub fn get_or_create_global(module: &Module, name: &str, val_type: &BasicType) -> GlobalValue {
    if let Some(global) = module.get_global(name) {
        global
    } else {
        module.add_global(val_type, None, name)
    }
}

pub fn get_const_vec(context: &Context, left: f64, right: f64) -> VectorValue {
    VectorType::const_vector(&[
        &context.f64_type().const_float(left),
        &context.f64_type().const_float(right),
    ])
}

pub fn get_vec_spread(context: &Context, val: f64) -> VectorValue {
    get_const_vec(context, val, val)
}

pub fn splat_vector(builder: &Builder, val: FloatValue, name: &str) -> VectorValue {
    let context = val.get_type().get_context();
    builder
        .build_insert_element(
            &builder
                .build_insert_element(
                    &context.f64_type().vec_type(2).get_undef(),
                    &val,
                    &context.i32_type().const_int(0, false),
                    name,
                )
                .into_vector_value(),
            &val,
            &context.i32_type().const_int(1, false),
            name,
        )
        .into_vector_value()
}

pub fn copy_ptr(builder: &mut Builder, module: &Module, src: PointerValue, dest: PointerValue) {
    let src_elem_type = src.get_type().element_type();
    let dest_elem_type = dest.get_type().element_type();
    assert_eq!(src_elem_type, dest_elem_type);

    let param_size = get_size_of(&src_elem_type).unwrap();
    let context = module.get_context();

    // cast src and dest to the correct pointer types
    let nullptr_type = context.i8_type().ptr_type(AddressSpace::Generic);
    let src_p0i8 = builder.build_pointer_cast(src, nullptr_type, "");
    let dest_p0i8 = builder.build_pointer_cast(dest, nullptr_type, "");

    builder.build_call(
        &intrinsics::memcpy(module),
        &[
            &dest_p0i8,
            &src_p0i8,
            &param_size,
            &context.i32_type().const_int(0, false),
            &context.bool_type().const_int(0, false),
        ],
        "",
        false,
    );
}

pub fn get_bit(builder: &mut Builder, bitmap: IntValue, bit: IntValue) -> IntValue {
    let bitmask = builder.build_left_shift(
        bitmap.get_type().const_int(1, false),
        builder.build_int_z_extend(bit, bitmap.get_type(), "bit"),
        "bitmask",
    );
    let active_bits = builder.build_and(bitmap, bitmask, "activebits");
    builder.build_int_compare(
        IntPredicate::NE,
        active_bits,
        bitmap.get_type().const_int(0, false),
        "activebit",
    )
}

pub fn set_bit(builder: &mut Builder, bitmap: IntValue, bit: IntValue) -> IntValue {
    let bitmask = builder.build_left_shift(
        bitmap.get_type().const_int(1, false),
        builder.build_int_z_extend(bit, bitmap.get_type(), "bit"),
        "bitmask",
    );
    builder.build_or(bitmap, bitmask, "setbit")
}

pub fn clear_bit(builder: &mut Builder, bitmap: IntValue, bit: IntValue) -> IntValue {
    let bitmask = builder.build_not(
        &builder.build_left_shift(
            bitmap.get_type().const_int(1, false),
            builder.build_int_z_extend(bit, bitmap.get_type(), "bit"),
            "",
        ),
        "bitmask",
    );
    builder.build_and(bitmap, bitmask, "clearbit")
}
