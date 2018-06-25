use codegen::values::{NumValue, TupleValue};
use codegen::BlockContext;
use inkwell::context::Context;
use inkwell::values::{BasicValue, BasicValueEnum, PointerValue};
use mir::ConstantValue;

pub fn gen_constant_statement(value: &ConstantValue, node: &mut BlockContext) -> PointerValue {
    let const_val = get_constant_val(node.ctx.context, value);
    let alloca = node.ctx
        .allocb
        .build_alloca(&const_val.get_type(), "constant");
    node.ctx.allocb.build_store(&alloca, &const_val);
    alloca
}

fn get_constant_val(context: &Context, value: &ConstantValue) -> BasicValueEnum {
    match value {
        ConstantValue::Num(num) => BasicValueEnum::from(NumValue::get_const(
            context,
            num.left,
            num.right,
            num.form as u8,
        )),
        ConstantValue::Tuple(tuple) => {
            let values: Vec<_> = tuple
                .items
                .iter()
                .map(|val| get_constant_val(context, val))
                .collect();
            let value_refs: Vec<_> = values.iter().map(|val| val as &BasicValue).collect();
            BasicValueEnum::from(TupleValue::get_const(context, &value_refs))
        }
    }
}
