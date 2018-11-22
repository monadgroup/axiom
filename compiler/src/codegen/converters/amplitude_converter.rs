use super::ConvertGenerator;
use ast::FormType;
use codegen::{math, util};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::VectorValue;

pub fn amplitude(generator: &mut ConvertGenerator) {
    generator.generate(FormType::Db, &amplitude_from_db);
}

fn amplitude_from_db(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    let exp10_intrinsic = math::exp10_v2f64(module);
    builder
        .build_call(
            &exp10_intrinsic,
            &[&builder.build_float_div(val, util::get_vec_spread(context, 20.), "")],
            "",
            true,
        ).left()
        .unwrap()
        .into_vector_value()
}
