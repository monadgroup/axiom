use super::ConvertGenerator;
use crate::ast::FormType;
use crate::codegen::{math, util};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::VectorValue;

pub fn q(generator: &mut ConvertGenerator) {
    generator.generate(FormType::Control, &q_from_control);
}

fn q_from_control(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    let pow_intrinsic = math::pow_v2f64(module);

    // todo: maybe just use x*x*x instead of pow(x, 3)?
    builder.build_float_add(
        builder.build_float_mul(
            util::get_vec_spread(context, 12. - 1. / 3.),
            builder
                .build_call(
                    &pow_intrinsic,
                    &[&val, &util::get_vec_spread(context, 3.)],
                    "",
                    true,
                )
                .left()
                .unwrap()
                .into_vector_value(),
            "",
        ),
        util::get_vec_spread(context, 1. / 3.),
        "",
    )
}
