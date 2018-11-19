use super::{Function, FunctionContext, VarArgs};
use codegen::util;
use codegen::values::NumValue;
use inkwell::module::Linkage;
use inkwell::types::BasicType;
use inkwell::values::{BasicValue, PointerValue};
use mir::block;
use std::iter;

fn gen_scalar_call(
    func: &mut FunctionContext,
    args: &[PointerValue],
    result: PointerValue,
    name: &str,
) {
    let call_func = util::get_or_create_func(func.ctx.module, name, true, &|| {
        let f64_type = func.ctx.context.f64_type();
        let param_types: Vec<_> = iter::repeat(&f64_type as &BasicType)
            .take(args.len())
            .collect();
        (
            Linkage::ExternalLinkage,
            func.ctx.context.f64_type().fn_type(&param_types, false),
        )
    });

    let result_num = NumValue::new(result);
    let result_form = NumValue::new(args[0]).get_form(func.ctx.b);
    result_num.set_form(func.ctx.b, &result_form);

    let vec_values: Vec<_> = args
        .iter()
        .map(|ptr| NumValue::new(*ptr).get_vec(func.ctx.b))
        .collect();
    let const_left_index = func.ctx.context.i32_type().const_int(0, false);
    let const_right_index = func.ctx.context.i32_type().const_int(1, false);

    let left_values: Vec<_> = vec_values
        .iter()
        .map(|vec| {
            func.ctx
                .b
                .build_extract_element(vec, &const_left_index, "arg.left")
                .into_float_value()
        }).collect();
    let left_basic_values: Vec<_> = left_values.iter().map(|val| val as &BasicValue).collect();
    let left_result = func
        .ctx
        .b
        .build_call(&call_func, &left_basic_values, "result.left", false)
        .left()
        .unwrap();

    let right_values: Vec<_> = vec_values
        .iter()
        .map(|vec| {
            func.ctx
                .b
                .build_extract_element(vec, &const_right_index, "arg.right")
                .into_float_value()
        }).collect();
    let right_basic_values: Vec<_> = right_values.iter().map(|val| val as &BasicValue).collect();
    let right_result = func
        .ctx
        .b
        .build_call(&call_func, &right_basic_values, "result.right", false)
        .left()
        .unwrap();

    let result_vec = func
        .ctx
        .b
        .build_insert_element(
            &func.ctx.context.f64_type().vec_type(2).get_undef(),
            &left_result,
            &const_left_index,
            "vec.withleft",
        ).into_vector_value();
    let result_vec = func
        .ctx
        .b
        .build_insert_element(&result_vec, &right_result, &const_right_index, "vec")
        .into_vector_value();
    result_num.set_vec(func.ctx.b, &result_vec);
}

macro_rules! define_scalar_intrinsic (
    ($func_name:ident: $func_type:expr => $intrinsic_name:expr) => (
        pub struct $func_name {}
        impl Function for $func_name {
            fn function_type() -> block::Function { $func_type }
            fn gen_call(func: &mut FunctionContext, args: &[PointerValue], _varargs: Option<VarArgs>, result: PointerValue) {
                gen_scalar_call(func, args, result, $intrinsic_name)
            }
        }
    )
);

define_scalar_intrinsic!(AcosFunction: block::Function::Acos => "acos");
define_scalar_intrinsic!(AsinFunction: block::Function::Asin => "asin");
define_scalar_intrinsic!(AtanFunction: block::Function::Atan => "atan");
define_scalar_intrinsic!(Atan2Function: block::Function::Atan2 => "atan2");
define_scalar_intrinsic!(HypotFunction: block::Function::Hypot => "hypot");
