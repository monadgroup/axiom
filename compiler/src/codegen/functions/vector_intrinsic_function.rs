use super::{Function, FunctionContext, VarArgs};
use ast::FormType;
use codegen::math;
use codegen::values::NumValue;
use inkwell::values::{BasicValue, FunctionValue, PointerValue};
use mir::block;

fn gen_intrinsic_call(
    func: &mut FunctionContext,
    args: &[PointerValue],
    result: PointerValue,
    intrinsic: FunctionValue,
    form: Option<FormType>,
) {
    let result_num = NumValue::new(result);
    let result_form = match form {
        Some(form) => func.ctx.context.i8_type().const_int(form as u64, false),
        None => NumValue::new(args[0]).get_form(func.ctx.b),
    };
    result_num.set_form(func.ctx.b, &result_form);

    let vec_values: Vec<_> = args
        .iter()
        .map(|ptr| NumValue::new(*ptr).get_vec(func.ctx.b))
        .collect();
    let basic_values: Vec<_> = vec_values.iter().map(|vec| vec as &BasicValue).collect();
    let result_vec = func
        .ctx
        .b
        .build_call(&intrinsic, &basic_values, "result", true)
        .left()
        .unwrap()
        .into_vector_value();
    result_num.set_vec(func.ctx.b, &result_vec);
}

macro_rules! define_vector_intrinsic (
    ($func_name:ident: $func_type:expr => $intrinsic_name:expr) => (
        pub struct $func_name {}
        impl Function for $func_name {
            fn function_type() -> block::Function { $func_type }
            fn gen_call(func: &mut FunctionContext, args: &[PointerValue], _varargs: Option<VarArgs>, result: PointerValue) {
                let intrinsic = $intrinsic_name(func.ctx.module);
                gen_intrinsic_call(func, args, result, intrinsic, None)
            }
        }
    );
    ($func_name:ident: $func_type:expr => $intrinsic_name:expr => $form_name:ident) => (
        pub struct $func_name {}
        impl Function for $func_name {
            fn function_type() -> block::Function { $func_type }
            fn gen_call(func: &mut FunctionContext, args: &[PointerValue], _varargs: Option<VarArgs>, result: PointerValue) {
                let intrinsic = $intrinsic_name(func.ctx.module);
                gen_intrinsic_call(func, args, result, intrinsic, Some(FormType::$form_name))
            }
        }
    )
);

define_vector_intrinsic!(NoiseFunction: block::Function::Noise => math::rand_v2f64 => Oscillator);
define_vector_intrinsic!(SinFunction: block::Function::Sin => math::sin_v2f64);
define_vector_intrinsic!(CosFunction: block::Function::Cos => math::cos_v2f64);
define_vector_intrinsic!(TanFunction: block::Function::Tan => math::tan_v2f64);
define_vector_intrinsic!(MinFunction: block::Function::Min => math::min_v2f64);
define_vector_intrinsic!(MaxFunction: block::Function::Max => math::max_v2f64);
define_vector_intrinsic!(SqrtFunction: block::Function::Sqrt => math::sqrt_v2f64);
define_vector_intrinsic!(FloorFunction: block::Function::Floor => math::floor_v2f64);
define_vector_intrinsic!(CeilFunction: block::Function::Ceil => math::ceil_v2f64);
define_vector_intrinsic!(RoundFunction: block::Function::Round => math::round_v2f64);
define_vector_intrinsic!(AbsFunction: block::Function::Abs => math::abs_v2f64);
define_vector_intrinsic!(CopySignFunction: block::Function::CopySign => math::copysign_v2f64);
define_vector_intrinsic!(FractFunction: block::Function::Fract => math::fract_v2f64);
define_vector_intrinsic!(ExpFunction: block::Function::Exp => math::exp_v2f64);
define_vector_intrinsic!(Exp2Function: block::Function::Exp2 => math::exp2_v2f64);
define_vector_intrinsic!(Exp10Function: block::Function::Exp10 => math::exp10_v2f64);
define_vector_intrinsic!(LogFunction: block::Function::Log => math::log_v2f64);
define_vector_intrinsic!(Log2Function: block::Function::Log2 => math::log2_v2f64);
define_vector_intrinsic!(Log10Function: block::Function::Log10 => math::log10_v2f64);
define_vector_intrinsic!(AsinFunction: block::Function::Asin => math::asin_v2f64);
define_vector_intrinsic!(AcosFunction: block::Function::Acos => math::acos_v2f64);
define_vector_intrinsic!(AtanFunction: block::Function::Atan => math::atan_v2f64);
define_vector_intrinsic!(Atan2Function: block::Function::Atan2 => math::atan2_v2f64);
define_vector_intrinsic!(SinhFunction: block::Function::Sinh => math::sinh_v2f64);
define_vector_intrinsic!(CoshFunction: block::Function::Cosh => math::cosh_v2f64);
define_vector_intrinsic!(TanhFunction: block::Function::Tanh => math::tanh_v2f64);
define_vector_intrinsic!(HypotFunction: block::Function::Hypot => math::hypot_v2f64);
