use super::{Function, FunctionContext, VarArgs};
use codegen::intrinsics;
use codegen::values::NumValue;
use inkwell::values::{BasicValue, FunctionValue, PointerValue};
use mir::block;

fn gen_intrinsic_call(
    func: &mut FunctionContext,
    args: &[PointerValue],
    result: PointerValue,
    intrinsic: FunctionValue,
) {
    let result_num = NumValue::new(result);
    let result_form = NumValue::new(args[0]).get_form(func.ctx.b);
    result_num.set_form(func.ctx.b, &result_form);

    let vec_values: Vec<_> = args
        .iter()
        .map(|ptr| NumValue::new(*ptr).get_vec(func.ctx.b))
        .collect();
    let basic_values: Vec<_> = vec_values.iter().map(|vec| vec as &BasicValue).collect();
    let result_vec = func
        .ctx
        .b
        .build_call(&intrinsic, &basic_values, "result", false)
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
                gen_intrinsic_call(func, args, result, intrinsic)
            }
        }
    );
);

define_vector_intrinsic!(CosFunction: block::Function::Cos => intrinsics::cos_v2f32);
define_vector_intrinsic!(SinFunction: block::Function::Sin => intrinsics::sin_v2f32);
define_vector_intrinsic!(LogFunction: block::Function::Log => intrinsics::log_v2f32);
define_vector_intrinsic!(Log2Function: block::Function::Log2 => intrinsics::log2_v2f32);
define_vector_intrinsic!(Log10Function: block::Function::Log10 => intrinsics::log10_v2f32);
define_vector_intrinsic!(SqrtFunction: block::Function::Sqrt => intrinsics::sqrt_v2f32);
define_vector_intrinsic!(CeilFunction: block::Function::Ceil => intrinsics::ceil_v2f32);
define_vector_intrinsic!(FloorFunction: block::Function::Floor => intrinsics::floor_v2f32);
define_vector_intrinsic!(AbsFunction: block::Function::Abs => intrinsics::fabs_v2f32);
define_vector_intrinsic!(ExpFunction: block::Function::Exp => intrinsics::exp_v2f32);
define_vector_intrinsic!(Exp2Function: block::Function::Exp2 => intrinsics::exp2_v2f32);
define_vector_intrinsic!(MinFunction: block::Function::Min => intrinsics::minnum_v2f32);
define_vector_intrinsic!(MaxFunction: block::Function::Max => intrinsics::maxnum_v2f32);
