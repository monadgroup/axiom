use super::BlockContext;
use crate::ast::ControlField;
use crate::codegen::controls;
use inkwell::values::PointerValue;

pub fn gen_load_control_statement(
    control: usize,
    field: ControlField,
    node: &mut BlockContext,
) -> PointerValue {
    let ptrs = node.get_control_ptrs(control);

    // allocate data for the field output
    let field_type = controls::get_field_type(node.ctx.context, field);
    let result_ptr = node.ctx.allocb.build_alloca(&field_type, "control.field");

    controls::build_field_get(node.ctx.module, node.ctx.b, field, ptrs, result_ptr);
    result_ptr
}
