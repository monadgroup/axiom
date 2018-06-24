use codegen::values::{NumValue, TupleValue};
use codegen::NodeContext;
use inkwell::context::Context;
use inkwell::values::{BasicValue, BasicValueEnum, PointerValue};
use mir::block::Function;
use mir::ConstantValue;

pub fn gen_call_func_statement(
    function: &Function,
    args: &[usize],
    varargs: &[usize],
    node: &mut NodeContext,
) -> PointerValue {
    unimplemented!()
}
