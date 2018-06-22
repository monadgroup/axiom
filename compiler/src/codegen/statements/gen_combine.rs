use codegen::values::TupleValue;
use codegen::NodeContext;
use inkwell::values::PointerValue;

pub fn gen_combine_statement(indexes: &[usize], node: &mut NodeContext) -> PointerValue {
    let pointer_vals: Vec<_> = indexes
        .iter()
        .map(|&index| node.get_statement(index))
        .collect();
    let new_tuple = TupleValue::new_from(
        node.module,
        node.alloca_builder,
        node.builder,
        &pointer_vals,
    );
    new_tuple.val
}
