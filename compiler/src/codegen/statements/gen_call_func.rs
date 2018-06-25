use codegen::BlockContext;
use inkwell::values::PointerValue;
use mir::block::Function;

pub fn gen_call_func_statement(
    _function: &Function,
    _args: &[usize],
    _varargs: &[usize],
    _node: &mut BlockContext,
) -> PointerValue {
    unimplemented!()
}
