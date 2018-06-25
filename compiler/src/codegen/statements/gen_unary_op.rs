use ast::UnaryOperation;
use codegen::util;
use codegen::values::NumValue;
use codegen::BlockContext;
use inkwell::values::PointerValue;
use inkwell::FloatPredicate;

pub fn gen_unary_op_statement(
    op: &UnaryOperation,
    input: usize,
    node: &mut BlockContext,
) -> PointerValue {
    let base_num = NumValue::new(node.get_statement(input));
    let new_num = NumValue::new_copy(node.ctx.module, node.ctx.allocb, node.ctx.b, &base_num);
    match op {
        UnaryOperation::Positive => {}
        UnaryOperation::Negative => {
            let original_vec = new_num.get_vec(node.ctx.b);
            let new_vec = node.ctx.b.build_float_neg(&original_vec, "num.vec.negate");
            new_num.set_vec(node.ctx.b, &new_vec);
        }
        UnaryOperation::Not => {
            let original_vec = new_num.get_vec(node.ctx.b);
            let zero_vec = util::get_vec_spread(node.ctx.context, 0.);
            let int_not = node.ctx.b.build_float_compare(
                FloatPredicate::OEQ,
                original_vec,
                zero_vec,
                "num.vec.not.int",
            );
            let float_not = node.ctx.b.build_unsigned_int_to_float(
                int_not,
                node.ctx.context.f32_type().vec_type(2),
                "num.vec.not",
            );
            new_num.set_vec(node.ctx.b, &float_not);
        }
    }
    new_num.val
}
