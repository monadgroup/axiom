use super::BlockContext;
use ast::FormType;
use codegen::values::NumValue;
use inkwell::values::PointerValue;

pub fn gen_num_cast_statement(
    target_form: &FormType,
    input: usize,
    node: &mut BlockContext,
) -> PointerValue {
    let base_num = NumValue::new(node.get_statement(input));
    let new_num = NumValue::new_copy(node.ctx.module, node.ctx.allocb, node.ctx.b, &base_num);
    new_num.set_form(
        node.ctx.b,
        &node
            .ctx
            .context
            .i8_type()
            .const_int(*target_form as u64, false),
    );
    new_num.val
}
