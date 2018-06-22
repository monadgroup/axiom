use super::ConvertGenerator;
use ast::FormType;
use codegen::intrinsics;
use codegen::{globals, util};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::{BasicValueEnum, VectorValue};
use std::f32::consts;

pub fn control(generator: &mut ConvertGenerator) {
    generator.generate(FormType::Beats, &control_from_beats);
    generator.generate(FormType::Db, &control_from_db);
    generator.generate(FormType::Frequency, &control_from_frequency);
    generator.generate(FormType::Note, &control_from_note);
    generator.generate(FormType::Oscillator, &control_from_oscillator);
    generator.generate(FormType::Q, &control_from_q);
    generator.generate(FormType::Samples, &control_from_samples);
    generator.generate(FormType::Seconds, &control_from_seconds);
}

fn control_from_beats(
    context: &Context,
    _module: &Module,
    builder: &mut Builder,
    val: &VectorValue,
) -> VectorValue {
    builder.build_float_div(val, &util::get_vec_spread(context, 8.), "")
}

fn control_from_db(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: &VectorValue,
) -> VectorValue {
    let pow_intrinsic = intrinsics::pow_v2f32(module);

    builder.build_float_div(
        &builder
            .build_call(
                &pow_intrinsic,
                &[
                    &BasicValueEnum::from(util::get_vec_spread(context, 10.)),
                    &BasicValueEnum::from(builder.build_float_div(
                        val,
                        &util::get_vec_spread(context, 20.),
                        "",
                    )),
                ],
                "",
                false,
            )
            .left()
            .unwrap()
            .into_vector_value(),
        &util::get_vec_spread(context, 2.),
        "",
    )
}

fn control_from_frequency(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: &VectorValue,
) -> VectorValue {
    let log_intrinsic = intrinsics::log_v2f32(module);

    builder.build_float_div(
        &builder
            .build_call(&log_intrinsic, &[val], "", false)
            .left()
            .unwrap()
            .into_vector_value(),
        &util::get_vec_spread(context, (20000 as f32).log(consts::E)),
        "",
    )
}

fn control_from_note(
    context: &Context,
    _module: &Module,
    builder: &mut Builder,
    val: &VectorValue,
) -> VectorValue {
    builder.build_float_div(val, &util::get_vec_spread(context, 127.), "")
}

fn control_from_oscillator(
    context: &Context,
    _module: &Module,
    builder: &mut Builder,
    val: &VectorValue,
) -> VectorValue {
    builder.build_float_add(
        &builder.build_float_mul(val, &util::get_vec_spread(context, 0.5), ""),
        &util::get_vec_spread(context, 0.5),
        "",
    )
}

fn control_from_q(
    context: &Context,
    _module: &Module,
    builder: &mut Builder,
    val: &VectorValue,
) -> VectorValue {
    builder.build_float_div(
        &builder.build_float_sub(val, &util::get_vec_spread(context, 0.5), ""),
        &builder.build_float_mul(val, &util::get_vec_spread(context, 0.999), ""),
        "",
    )
}

fn control_from_samples(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: &VectorValue,
) -> VectorValue {
    builder.build_float_div(
        &builder.build_float_mul(val, &util::get_vec_spread(context, 1.1), ""),
        &builder.build_float_add(
            val,
            &builder.build_float_mul(
                &util::get_vec_spread(context, 0.1),
                &builder
                    .build_load(&globals::get_sample_rate(module), "samplerate")
                    .into_vector_value(),
                "",
            ),
            "",
        ),
        "",
    )
}

fn control_from_seconds(
    context: &Context,
    _module: &Module,
    builder: &mut Builder,
    val: &VectorValue,
) -> VectorValue {
    builder.build_float_div(
        &builder.build_float_mul(val, &util::get_vec_spread(context, 1.1), ""),
        &builder.build_float_add(val, &util::get_vec_spread(context, 0.5), ""),
        "",
    )
}
