use super::{Function, FunctionContext, VarArgs};
use codegen::values::NumValue;
use inkwell::context::Context;
use inkwell::types::StructType;
use inkwell::values::PointerValue;
use mir::block;

pub struct NextFunction {}
impl Function for NextFunction {
    fn function_type() -> block::Function {
        block::Function::Next
    }

    fn data_type(context: &Context) -> StructType {
        NumValue::get_type(context)
    }

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        _varargs: Option<VarArgs>,
        result: PointerValue,
    ) {
        let last_num = NumValue::new(func.data_ptr);
        let new_num = NumValue::new(args[0]);
        let result_num = NumValue::new(result);

        last_num.copy_to(func.ctx.b, func.ctx.module, &result_num);
        new_num.copy_to(func.ctx.b, func.ctx.module, &last_num);
    }
}

/*pub struct AmplitudeFunction {}
impl Function for AmplitudeFunction {

}

pub struct HoldFunction {}
impl Function for HoldFunction {

}

pub struct AccumFunction {}
impl Function for AccumFunction {

}*/
