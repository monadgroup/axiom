use super::BlockContext;
use ast::ControlField;
use codegen::controls;
use inkwell::values::PointerValue;
use inkwell::AddressSpace;

pub fn gen_store_control_statement(
    control: usize,
    field: &ControlField,
    value: usize,
    node: &mut BlockContext,
) -> PointerValue {
    let ptrs = node.get_control_ptrs(control, false);

    let store_val = node.get_statement(value);
    controls::build_field_set(
        node.ctx.module,
        node.ctx.b,
        *field,
        ptrs.value,
        ptrs.data,
        store_val,
    );

    // storing a control has no result, return an undefined value
    node.ctx
        .context
        .i8_type()
        .ptr_type(AddressSpace::Generic)
        .get_undef()
}
