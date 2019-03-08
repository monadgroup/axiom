use super::ConvertGenerator;
use crate::ast::FormType;
use crate::codegen::{globals, math, util};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::VectorValue;
use std::f64::consts;

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
    val: VectorValue,
) -> VectorValue {
    builder.build_float_div(val, util::get_vec_spread(context, 8.), "")
}

fn control_from_db(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    let exp10_intrinsic = math::exp10_v2f64(module);

    builder.build_float_div(
        builder
            .build_call(
                &exp10_intrinsic,
                &[&builder.build_float_div(val, util::get_vec_spread(context, 20.), "")],
                "",
                true,
            )
            .left()
            .unwrap()
            .into_vector_value(),
        util::get_vec_spread(context, 2.),
        "",
    )
}

fn control_from_frequency(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    let log_intrinsic = math::log_v2f64(module);

    builder.build_float_div(
        builder
            .build_call(
                &log_intrinsic,
                &[&builder.build_float_add(val, util::get_vec_spread(context, 1.), "")],
                "",
                true,
            )
            .left()
            .unwrap()
            .into_vector_value(),
        util::get_vec_spread(context, 20000_f64.log(consts::E)),
        "",
    )
}

fn control_from_note(
    context: &Context,
    _module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_div(val, util::get_vec_spread(context, 127.), "")
}

fn control_from_oscillator(
    context: &Context,
    _module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_add(
        builder.build_float_mul(val, util::get_vec_spread(context, 0.5), ""),
        util::get_vec_spread(context, 0.5),
        "",
    )
}

fn control_from_q(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    let pow_intrinsic = math::pow_v2f64(module);

    builder
        .build_call(
            &pow_intrinsic,
            &[
                &builder.build_float_div(
                    builder.build_float_sub(val, util::get_vec_spread(context, 1. / 3.), ""),
                    util::get_vec_spread(context, 12. - 1. / 3.),
                    "",
                ),
                &util::get_vec_spread(context, 1. / 3.),
            ],
            "",
            true,
        )
        .left()
        .unwrap()
        .into_vector_value()
}

fn control_from_samples(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_div(
        builder.build_float_mul(val, util::get_vec_spread(context, 1.1), ""),
        builder.build_float_add(
            val,
            builder.build_float_mul(
                util::get_vec_spread(context, 0.1),
                builder
                    .build_load(
                        &globals::get_sample_rate(module).as_pointer_value(),
                        "samplerate",
                    )
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
    val: VectorValue,
) -> VectorValue {
    builder.build_float_div(
        builder.build_float_mul(val, util::get_vec_spread(context, 1.1), ""),
        builder.build_float_add(val, util::get_vec_spread(context, 0.5), ""),
        "",
    )
}
