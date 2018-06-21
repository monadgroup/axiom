use ast::FormType;
use inkwell::values::PointerValue;
use codegen::NodeContext;
use codegen::values::NumValue;

pub fn gen_num_cast_statement(target_form: &FormType, input: usize, node: &mut NodeContext) -> PointerValue {
    let base_num = NumValue::new(node.get_statement(input));
    let new_num = NumValue::new_copy(node.module, node.alloca_builder, node.builder, &base_num);
    new_num.set_form(node.builder, &node.context.i8_type().const_int(*target_form as u64, false));
    new_num.val
}
