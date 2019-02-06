use super::ConvertGenerator;
use crate::ast::FormType;
use crate::codegen::util;
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::VectorValue;

pub fn q(generator: &mut ConvertGenerator) {
    generator.generate(FormType::Control, &q_from_control);
}

fn q_from_control(
    context: &Context,
    _module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    builder.build_float_div(
        util::get_vec_spread(context, -0.5),
        builder.build_float_sub(
            builder.build_float_mul(val, util::get_vec_spread(context, 0.999), ""),
            util::get_vec_spread(context, 1.),
            "",
        ),
        "",
    )
}
