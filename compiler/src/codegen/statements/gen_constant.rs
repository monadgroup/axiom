use mir::ConstantValue;
use codegen::NodeContext;
use codegen::values::{NumValue, TupleValue};
use inkwell::values::{PointerValue, BasicValueEnum, BasicValue};
use inkwell::context::Context;

pub fn gen_constant_statement(value: &ConstantValue, node: &mut NodeContext) -> PointerValue {
    let const_val = get_constant_val(node.context, value);
    let alloca = node.alloca_builder.build_alloca(&const_val.get_type(), "constant");
    node.alloca_builder.build_store(&alloca, &const_val);
    alloca
}

fn get_constant_val(context: &Context, value: &ConstantValue) -> BasicValueEnum {
    match value {
        ConstantValue::Num(num) => {
            BasicValueEnum::from(NumValue::get_const(context, num.left, num.right, num.form as u8))
        },
        ConstantValue::Tuple(tuple) => {
            let values: Vec<_> = tuple.items.iter().map(|val| get_constant_val(context, val)).collect();
            let value_refs: Vec<_> = values.iter().map(|val| val as &BasicValue).collect();
            BasicValueEnum::from(TupleValue::get_const(context, &value_refs))
        }
    }
}
