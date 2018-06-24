use super::ConvertGenerator;
use ast::FormType;
use codegen::{globals, util};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::VectorValue;

pub fn seconds(generator: &mut ConvertGenerator) {
    generator.generate(FormType::Beats, &seconds_from_beats);
    generator.generate(FormType::Control, &seconds_from_control);
    generator.generate(FormType::Frequency, &seconds_from_frequency);
    generator.generate(FormType::Samples, &seconds_from_samples);
}

fn seconds_from_beats(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_div(
        val,
        builder.build_float_div(
            builder
                .build_load(&globals::get_bpm(module), "bpm")
                .into_vector_value(),
            util::get_vec_spread(context, 60.),
            "",
        ),
        "",
    )
}

fn seconds_from_control(
    context: &Context,
    _module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_div(
        val,
        builder.build_float_sub(
            util::get_vec_spread(context, 2.2),
            builder.build_float_mul(val, util::get_vec_spread(context, 2.), ""),
            "",
        ),
        "",
    )
}

fn seconds_from_frequency(
    context: &Context,
    _module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_div(util::get_vec_spread(context, 1.), val, "")
}

fn seconds_from_samples(
    _context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_div(
        val,
        builder
            .build_load(&globals::get_sample_rate(module), "samplerate")
            .into_vector_value(),
        "",
    )
}
