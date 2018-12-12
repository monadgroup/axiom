use super::BlockContext;
use crate::codegen::functions;
use crate::mir::block::Function;
use inkwell::values::PointerValue;

pub fn gen_call_func_statement(
    index: usize,
    function: &Function,
    args: &[usize],
    varargs: &[usize],
    node: &mut BlockContext,
) -> PointerValue {
    let layout_index = node.layout.statement_index(index).unwrap();
    let func_data = node.get_function_ptr(layout_index);

    // allocate data for the function result
    let return_type = functions::get_return_type(node.ctx.context, *function);
    let return_ptr = node.ctx.allocb.build_alloca(&return_type, "func.return");

    let arg_ptrs: Vec<_> = args
        .iter()
        .map(|index| node.get_statement(*index))
        .collect();
    let vararg_ptrs: Vec<_> = varargs
        .iter()
        .map(|index| node.get_statement(*index))
        .collect();

    functions::build_call(
        &mut node.ctx,
        *function,
        func_data,
        arg_ptrs,
        vararg_ptrs,
        return_ptr,
    );
    return_ptr
}
