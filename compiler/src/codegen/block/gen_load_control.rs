use super::BlockContext;
use ast::ControlField;
use codegen::controls;
use inkwell::values::PointerValue;

pub fn gen_load_control_statement(
    control: usize,
    field: &ControlField,
    node: &mut BlockContext,
) -> PointerValue {
    let ptrs = node.get_control_ptrs(control, false);

    // allocate data for the field output
    let field_type = controls::get_field_type(node.ctx.context, *field);
    let result_ptr = node.ctx.allocb.build_alloca(&field_type, "control.field");

    controls::build_field_get(
        node.ctx.module,
        node.ctx.b,
        *field,
        ptrs.value,
        ptrs.data,
        ptrs.shared,
        result_ptr,
    );
    result_ptr
}
