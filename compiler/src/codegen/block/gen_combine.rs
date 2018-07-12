use super::BlockContext;
use codegen::values::TupleValue;
use inkwell::values::PointerValue;

pub fn gen_combine_statement(indexes: &[usize], node: &mut BlockContext) -> PointerValue {
    let pointer_vals: Vec<_> = indexes
        .iter()
        .map(|&index| node.get_statement(index))
        .collect();
    let new_tuple =
        TupleValue::new_from(node.ctx.module, node.ctx.allocb, node.ctx.b, &pointer_vals);
    new_tuple.val
}
