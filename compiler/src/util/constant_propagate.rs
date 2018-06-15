use ast::{FormType, OperatorType, SourceRange, UnaryOperation};
use mir::block::{ConstantNum, ConstantTuple, ConstantValue, Function};
use {CompileError, CompileResult};

pub fn const_convert(constant: &ConstantNum, target_form: &FormType) -> ConstantNum {
    unimplemented!();
}

pub fn const_cast(constant: &ConstantNum, target_form: FormType) -> ConstantNum {
    constant.with_form(target_form)
}

pub fn const_unary_op(constant: &ConstantNum, op: UnaryOperation) -> ConstantNum {
    match op {
        UnaryOperation::Positive => constant.clone(),
        UnaryOperation::Negative => {
            ConstantNum::new(-constant.left, -constant.right, constant.form)
        }
        UnaryOperation::Not => ConstantNum::new(
            f32::from((constant.left == 0.) as i8),
            f32::from((constant.right == 0.) as i8),
            constant.form,
        ),
    }
}

#[allow(float_cmp)]
pub fn const_math_op(a: &ConstantNum, b: &ConstantNum, op: OperatorType) -> ConstantNum {
    match op {
        OperatorType::Identity => b.clone(),
        OperatorType::Add => ConstantNum::new(a.left + b.left, a.right + b.right, a.form),
        OperatorType::Subtract => ConstantNum::new(a.left - b.left, a.right - b.right, a.form),
        OperatorType::Multiply => ConstantNum::new(a.left * b.left, a.right * b.right, a.form),
        OperatorType::Divide => ConstantNum::new(a.left / b.left, a.right / b.right, a.form),
        OperatorType::Modulo => ConstantNum::new(a.left % b.left, a.right % b.right, a.form),
        OperatorType::Power => ConstantNum::new(a.left.powf(b.left), a.right.powf(b.right), a.form),
        OperatorType::BitwiseAnd => ConstantNum::new(
            (a.left as i32 & b.left as i32) as f32,
            (a.right as i32 & b.right as i32) as f32,
            a.form,
        ),
        OperatorType::BitwiseOr => ConstantNum::new(
            (a.left as i32 | b.left as i32) as f32,
            (a.right as i32 | b.right as i32) as f32,
            a.form,
        ),
        OperatorType::BitwiseXor => ConstantNum::new(
            (a.left as i32 ^ b.left as i32) as f32,
            (a.right as i32 ^ b.right as i32) as f32,
            a.form,
        ),
        OperatorType::LogicalAnd => ConstantNum::new(
            f32::from((a.left != 0. && b.left != 0.) as i8),
            f32::from((a.right != 0. && b.right != 0.) as i8),
            a.form,
        ),
        OperatorType::LogicalOr => ConstantNum::new(
            f32::from((a.left != 0. || b.left != 0.) as i8),
            f32::from((a.right != 0. || b.right != 0.) as i8),
            a.form,
        ),
        OperatorType::LogicalEqual => ConstantNum::new(
            f32::from((a.left == b.left) as i8),
            f32::from((a.right == b.right) as i8),
            a.form,
        ),
        OperatorType::LogicalNotEqual => ConstantNum::new(
            f32::from((a.left != b.left) as i8),
            f32::from((a.right != b.right) as i8),
            a.form,
        ),
        OperatorType::LogicalGt => ConstantNum::new(
            f32::from((a.left > b.left) as i8),
            f32::from((a.right > b.right) as i8),
            a.form,
        ),
        OperatorType::LogicalLt => ConstantNum::new(
            f32::from((a.left < b.left) as i8),
            f32::from((a.right < b.right) as i8),
            a.form,
        ),
        OperatorType::LogicalGte => ConstantNum::new(
            f32::from((a.left >= b.left) as i8),
            f32::from((a.right >= b.right) as i8),
            a.form,
        ),
        OperatorType::LogicalLte => ConstantNum::new(
            f32::from((a.left <= b.left) as i8),
            f32::from((a.right <= b.right) as i8),
            a.form,
        ),
    }
}

pub fn const_extract<'a>(
    tuple: &'a ConstantTuple,
    index: usize,
    range: &SourceRange,
) -> CompileResult<&'a ConstantValue> {
    if index >= tuple.items.len() {
        Err(CompileError::access_out_of_bounds(
            tuple.items.len(),
            index,
            *range,
        ))
    } else {
        Ok(&tuple.items[index])
    }
}

pub fn const_combine(values: Vec<ConstantValue>) -> ConstantTuple {
    ConstantTuple { items: values }
}

pub fn const_call(
    function: &Function,
    args: &[ConstantValue],
) -> Option<CompileResult<ConstantValue>> {
    unimplemented!();
}
