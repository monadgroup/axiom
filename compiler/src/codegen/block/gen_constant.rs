use super::BlockContext;
use crate::codegen::values::remap_constant;
use crate::mir::ConstantValue;
use inkwell::values::PointerValue;

pub fn gen_constant_statement(value: &ConstantValue, node: &mut BlockContext) -> PointerValue {
    let const_val = remap_constant(node.ctx.context, value);
    let alloca = node
        .ctx
        .allocb
        .build_alloca(&const_val.get_type(), "constant");
    node.ctx.allocb.build_store(&alloca, &const_val);
    alloca
}
