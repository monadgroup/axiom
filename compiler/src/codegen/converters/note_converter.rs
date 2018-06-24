use super::ConvertGenerator;
use ast::FormType;
use codegen::{intrinsics, util};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::VectorValue;

pub fn note(generator: &mut ConvertGenerator) {
    generator.generate(FormType::Control, &note_from_control);
    generator.generate(FormType::Frequency, &note_from_frequency);
}

fn note_from_control(
    context: &Context,
    _module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_mul(val, util::get_vec_spread(context, 127.), "")
}

fn note_from_frequency(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    let log2_intrinsic = intrinsics::log2_v2f32(module);

    builder.build_float_add(
        util::get_vec_spread(context, 69.),
        builder.build_float_mul(
            util::get_vec_spread(context, 12.),
            builder
                .build_call(
                    &log2_intrinsic,
                    &[&builder.build_float_div(val, util::get_vec_spread(context, 440.), "")],
                    "",
                    false,
                )
                .left()
                .unwrap()
                .into_vector_value(),
            "",
        ),
        "",
    )
}
