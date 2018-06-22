mod gen_combine;
mod gen_constant;
mod gen_extract;
mod gen_math_op;
mod gen_num_cast;
mod gen_num_convert;
mod gen_unary_op;

use codegen::NodeContext;
use inkwell::values::PointerValue;
use mir::block::Statement;

use self::gen_combine::gen_combine_statement;
use self::gen_constant::gen_constant_statement;
use self::gen_extract::gen_extract_statement;
use self::gen_math_op::gen_math_op_statement;
use self::gen_num_cast::gen_num_cast_statement;
use self::gen_unary_op::gen_unary_op_statement;

pub fn gen_statement(statement: &Statement, node: &mut NodeContext) -> PointerValue {
    match statement {
        Statement::Constant(constant) => gen_constant_statement(constant, node),
        Statement::NumCast { target_form, input } => {
            gen_num_cast_statement(target_form, *input, node)
        }
        Statement::NumUnaryOp { op, input } => gen_unary_op_statement(op, *input, node),
        Statement::NumMathOp { op, lhs, rhs } => gen_math_op_statement(op, *lhs, *rhs, node),
        Statement::Extract { tuple, index } => gen_extract_statement(*tuple, *index, node),
        Statement::Combine { indexes } => gen_combine_statement(indexes, node),
        _ => unimplemented!(),
    }
}
