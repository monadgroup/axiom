use super::{Function, FunctionContext, VarArgs};
use codegen::intrinsics;
use codegen::values::NumValue;
use inkwell::module::Module;
use inkwell::types::VectorType;
use inkwell::values::{BasicValue, FunctionValue, PointerValue};
use mir::block;

fn gen_shuffle_call(
    func: &mut FunctionContext,
    args: &[PointerValue],
    result: PointerValue,
    shuffle: (i32, i32),
) {
    let input_num = NumValue::new(args[0]);
    let result_num = NumValue::new(result);
    let result_form = input_num.get_form(func.ctx.b);
    result_num.set_form(func.ctx.b, &result_form);

    let input_vec = input_num.get_vec(func.ctx.b);
    let shuffle_vec = VectorType::const_vector(&[
        &func.ctx
            .context
            .i32_type()
            .const_int(shuffle.0 as u64, false),
        &func.ctx
            .context
            .i32_type()
            .const_int(shuffle.1 as u64, false),
    ]);
    let shuffled_vec = func.ctx.b.build_shuffle_vector(
        &input_vec,
        &input_vec.get_type().get_undef(),
        &shuffle_vec,
        "",
    );
    result_num.set_vec(func.ctx.b, &shuffled_vec);
}

macro_rules! define_vector_shuffle (
    ($func_name:ident: $func_type:expr => ($shuffle_1:expr, $shuffle_2:expr)) => (
        pub struct $func_name {}
        impl Function for $func_name {
            fn function_type() -> block::Function { $func_type }
            fn gen_call(func: &mut FunctionContext, args: &[PointerValue], _varargs: Option<VarArgs>, result: PointerValue) {
                gen_shuffle_call(func, args, result, ($shuffle_1, $shuffle_2))
            }
        }
    )
);

define_vector_shuffle!(LeftFunction: block::Function::Left => (0, 0));
define_vector_shuffle!(RightFunction: block::Function::Right => (1, 1));
define_vector_shuffle!(SwapFunction: block::Function::Swap => (1, 0));
