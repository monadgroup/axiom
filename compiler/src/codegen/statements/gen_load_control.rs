use ast::ControlField;
use codegen::NodeContext;
use inkwell::values::PointerValue;

pub fn gen_load_control_statement(
    _control: usize,
    _field: &ControlField,
    _node: &mut NodeContext,
) -> PointerValue {
    unimplemented!()
}
