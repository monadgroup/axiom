use super::BlockContext;
use crate::codegen::values::TupleValue;
use inkwell::values::PointerValue;

pub fn gen_extract_statement(tuple: usize, index: usize, node: &mut BlockContext) -> PointerValue {
    let base_tuple = TupleValue::new(node.get_statement(tuple));
    base_tuple.get_item_ptr(node.ctx.b, index)
}
