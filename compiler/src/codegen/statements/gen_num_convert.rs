use ast::FormType;
use codegen::values::NumValue;
use codegen::NodeContext;
use inkwell::values::PointerValue;

pub fn gen_num_convert_statement(
    target_form: &FormType,
    input: usize,
    node: &mut NodeContext,
) -> PointerValue {
    let base_num = NumValue::new(node.get_statement(input));
    unimplemented!()
}
