use super::ConvertGenerator;
use ast::FormType;
use codegen::{intrinsics, util};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::{BasicValueEnum, VectorValue};

pub fn amplitude(generator: &mut ConvertGenerator) {
    generator.generate(FormType::Db, &amplitude_from_db);
}

fn amplitude_from_db(
    context: &Context,
    module: &Module,
    builder: &mut Builder,
    val: VectorValue,
) -> VectorValue {
    let pow_intrinsic = intrinsics::pow_v2f32(module);
    builder
        .build_call(
            &pow_intrinsic,
            &[
                &util::get_vec_spread(context, 10.),
                &builder.build_float_div(val, util::get_vec_spread(context, 20.), ""),
            ],
            "",
            false,
        )
        .left()
        .unwrap()
        .into_vector_value()
}
