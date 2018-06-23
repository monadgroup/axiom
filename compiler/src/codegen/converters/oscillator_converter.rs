use super::ConvertGenerator;
use ast::FormType;
use codegen::{globals, util};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::VectorValue;

pub fn oscillator(generator: &mut ConvertGenerator) {
    generator.generate(FormType::Control, &oscillator_from_control);
}

fn oscillator_from_control(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_sub(
        builder.build_float_mul(val, util::get_vec_spread(context, 2.), ""),
        util::get_vec_spread(context, 1.),
        "",
    )
}
