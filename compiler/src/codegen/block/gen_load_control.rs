use super::BlockContext;
use ast::ControlField;
use codegen::controls;
use inkwell::values::PointerValue;

pub fn gen_load_control_statement(
    control: usize,
    field: &ControlField,
    node: &mut BlockContext,
) -> PointerValue {
    let layout_index = node.layout.control_index(control);
    let control_data = node.get_data_entry(layout_index);
    let control_group = node.get_group_entry(layout_index);

    // allocate data for the field output
    let field_type = controls::get_field_type(node.ctx.context, *field);
    let result_ptr = node.ctx.allocb.build_alloca(&field_type, "control.field");

    controls::build_field_get(
        node.ctx.module,
        node.ctx.b,
        *field,
        control_group,
        control_data,
        result_ptr,
    );
    result_ptr
}
