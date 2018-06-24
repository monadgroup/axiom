use ast::ControlField;
use codegen::values::{NumValue, TupleValue};
use codegen::NodeContext;
use inkwell::context::Context;
use inkwell::values::{BasicValue, BasicValueEnum, PointerValue};
use mir::ConstantValue;

pub fn gen_store_control_statement(
    control: usize,
    field: &ControlField,
    value: usize,
    node: &mut NodeContext,
) -> PointerValue {
    unimplemented!()
}
