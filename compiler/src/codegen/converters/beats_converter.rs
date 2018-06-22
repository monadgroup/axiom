use super::ConvertGenerator;
use ast::FormType;
use codegen::{globals, util};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::VectorValue;

pub fn beats(generator: &mut ConvertGenerator) {
    generator.generate(FormType::Control, &beats_from_control);
    generator.generate(FormType::Frequency, &beats_from_frequency);
    generator.generate(FormType::Samples, &beats_from_samples);
    generator.generate(FormType::Seconds, &beats_from_seconds);
}

fn beats_from_control(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: &VectorValue,
) -> VectorValue {
    builder.build_float_mul(val, &util::get_vec_spread(context, 8.), "")
}

fn beats_from_frequency(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: &VectorValue,
) -> VectorValue {
    builder.build_float_div(
        &globals::get_bpm(module),
        &builder.build_float_mul(&util::get_vec_spread(context, 60.), val, ""),
        "",
    )
}

fn beats_from_samples(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: &VectorValue,
) -> VectorValue {
    builder.build_float_div(
        &builder.build_float_mul(val, &globals::get_bpm(module), ""),
        &builder.build_float_mul(
            &globals::get_sample_rate(module),
            &util::get_vec_spread(context, 60.),
            "",
        ),
        "",
    )
}

fn beats_from_seconds(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: &VectorValue,
) -> VectorValue {
    builder.build_float_mul(
        val,
        &builder.build_float_mul(
            &globals::get_bpm(module),
            &util::get_vec_spread(context, 60.),
            "",
        ),
        "",
    )
}
