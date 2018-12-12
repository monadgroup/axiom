use super::ConvertGenerator;
use crate::ast::FormType;
use crate::codegen::{globals, util};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::VectorValue;

pub fn samples(generator: &mut ConvertGenerator) {
    generator.generate(FormType::Beats, &samples_from_beats);
    generator.generate(FormType::Control, &samples_from_control);
    generator.generate(FormType::Frequency, &samples_from_frequency);
    generator.generate(FormType::Seconds, &samples_from_seconds);
}

fn samples_from_beats(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_div(
        builder.build_float_mul(
            val,
            builder.build_float_mul(
                builder
                    .build_load(
                        &globals::get_sample_rate(module).as_pointer_value(),
                        "samplerate",
                    )
                    .into_vector_value(),
                util::get_vec_spread(context, 60.),
                "",
            ),
            "",
        ),
        builder
            .build_load(&globals::get_bpm(module).as_pointer_value(), "bpm")
            .into_vector_value(),
        "",
    )
}

fn samples_from_control(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_div(
        builder.build_float_mul(
            val,
            builder
                .build_load(
                    &globals::get_sample_rate(module).as_pointer_value(),
                    "samplerate",
                )
                .into_vector_value(),
            "",
        ),
        builder.build_float_sub(
            util::get_vec_spread(context, 11.),
            builder.build_float_mul(val, util::get_vec_spread(context, 10.), ""),
            "",
        ),
        "",
    )
}

fn samples_from_frequency(
    _context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_div(
        builder
            .build_load(
                &globals::get_sample_rate(module).as_pointer_value(),
                "samplerate",
            )
            .into_vector_value(),
        val,
        "",
    )
}

fn samples_from_seconds(
    _context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_mul(
        val,
        builder
            .build_load(
                &globals::get_sample_rate(module).as_pointer_value(),
                "samplerate",
            )
            .into_vector_value(),
        "",
    )
}
