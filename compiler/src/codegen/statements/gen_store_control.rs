use ast::ControlField;
use codegen::NodeContext;
use inkwell::values::PointerValue;

pub fn gen_store_control_statement(
    _control: usize,
    _field: &ControlField,
    _value: usize,
    _node: &mut NodeContext,
) -> PointerValue {
    unimplemented!()
}
