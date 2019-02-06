use super::BlockContext;
use crate::ast::FormType;
use crate::codegen::converters;
use crate::codegen::values::NumValue;
use inkwell::values::PointerValue;

pub fn gen_num_convert_statement(
    target_form: FormType,
    input: usize,
    node: &mut BlockContext,
) -> PointerValue {
    let base_num = NumValue::new(node.get_statement(input));
    converters::build_convert(
        node.ctx.allocb,
        node.ctx.b,
        node.ctx.module,
        &base_num,
        target_form,
    )
    .val
}
