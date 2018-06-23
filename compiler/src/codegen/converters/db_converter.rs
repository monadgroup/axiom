use super::ConvertGenerator;
use ast::FormType;
use codegen::intrinsics;
use codegen::{globals, util};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::{BasicValueEnum, VectorValue};
use std::f32::consts;

pub fn db(generator: &mut ConvertGenerator) {
    generator.generate(FormType::Amplitude, &db_from_amplitude);
    generator.generate(FormType::Control, &db_from_control);
}

fn db_from_amplitude(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    let log10_intrinsic = intrinsics::log10_v2f32(module);

    builder.build_float_mul(
        builder
            .build_call(&log10_intrinsic, &[&val], "", false)
            .left()
            .unwrap()
            .into_vector_value(),
        util::get_vec_spread(context, 20.),
        "",
    )
}

fn db_from_control(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    let log10_intrinsic = intrinsics::log10_v2f32(module);

    builder.build_float_mul(
        builder
            .build_call(
                &log10_intrinsic,
                &[&builder.build_float_mul(val, util::get_vec_spread(context, 2.), "")],
                "",
                false,
            )
            .left()
            .unwrap()
            .into_vector_value(),
        util::get_vec_spread(context, 20.),
        "",
    )
}
