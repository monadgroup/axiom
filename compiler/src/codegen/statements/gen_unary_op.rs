use ast::UnaryOperation;
use codegen::util;
use codegen::values::NumValue;
use codegen::NodeContext;
use inkwell::values::InstructionOpcode;
use inkwell::values::PointerValue;
use inkwell::FloatPredicate;

pub fn gen_unary_op_statement(
    op: &UnaryOperation,
    input: usize,
    node: &mut NodeContext,
) -> PointerValue {
    let base_num = NumValue::new(node.get_statement(input));
    let new_num = NumValue::new_copy(node.module, node.alloca_builder, node.builder, &base_num);
    match op {
        UnaryOperation::Positive => {}
        UnaryOperation::Negative => {
            let original_vec = new_num.get_vec(node.builder);
            let new_vec = node.builder
                .build_float_neg(&original_vec, "num.vec.negate");
            new_num.set_vec(node.builder, &new_vec);
        }
        UnaryOperation::Not => {
            let original_vec = new_num.get_vec(node.builder);
            let zero_vec = util::get_vec_spread(node.context, 0.);
            let int_not = node.builder.build_float_compare(
                FloatPredicate::OEQ,
                &original_vec,
                &zero_vec,
                "num.vec.not.int",
            );
            let float_not = node.builder
                .build_cast(
                    InstructionOpcode::UIToFP,
                    &int_not,
                    &node.context.f32_type().vec_type(2),
                    "num.vec.not",
                )
                .into_vector_value();
            new_num.set_vec(node.builder, &float_not);
        }
    }
    new_num.val
}
