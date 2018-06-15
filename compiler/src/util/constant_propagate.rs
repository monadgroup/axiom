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
            ConstantNum::new(-constant.left, -constant.right, constant.form.clone())
        }
        UnaryOperation::Not => ConstantNum::new(
            (constant.left == 0.) as i8 as f32,
            (constant.right == 0.) as i8 as f32,
            constant.form.clone(),
        ),
    }
}

pub fn const_math_op(a: &ConstantNum, b: &ConstantNum, op: OperatorType) -> ConstantNum {
    match op {
        OperatorType::Identity => b.clone(),
        OperatorType::Add => ConstantNum::new(a.left + b.left, a.right + b.right, a.form.clone()),
        OperatorType::Subtract => {
            ConstantNum::new(a.left - b.left, a.right - b.right, a.form.clone())
        }
        OperatorType::Multiply => {
            ConstantNum::new(a.left * b.left, a.right * b.right, a.form.clone())
        }
        OperatorType::Divide => {
            ConstantNum::new(a.left / b.left, a.right / b.right, a.form.clone())
        }
        OperatorType::Modulo => {
            ConstantNum::new(a.left % b.left, a.right % b.right, a.form.clone())
        }
        OperatorType::Power => {
            ConstantNum::new(a.left.powf(b.left), a.right.powf(b.right), a.form.clone())
        }
        OperatorType::BitwiseAnd => ConstantNum::new(
            (a.left as i32 & b.left as i32) as f32,
            (a.right as i32 & b.right as i32) as f32,
            a.form.clone(),
        ),
        OperatorType::BitwiseOr => ConstantNum::new(
            (a.left as i32 | b.left as i32) as f32,
            (a.right as i32 | b.right as i32) as f32,
            a.form.clone(),
        ),
        OperatorType::BitwiseXor => ConstantNum::new(
            (a.left as i32 ^ b.left as i32) as f32,
            (a.right as i32 ^ b.right as i32) as f32,
            a.form.clone(),
        ),
        OperatorType::LogicalAnd => ConstantNum::new(
            (a.left != 0. && b.left != 0.) as i8 as f32,
            (a.right != 0. && b.right != 0.) as i8 as f32,
            a.form.clone(),
        ),
        OperatorType::LogicalOr => ConstantNum::new(
            (a.left != 0. || b.left != 0.) as i8 as f32,
            (a.right != 0. || b.right != 0.) as i8 as f32,
            a.form.clone(),
        ),
        OperatorType::LogicalEqual => ConstantNum::new(
            (a.left == b.left) as i8 as f32,
            (a.right == b.right) as i8 as f32,
            a.form.clone(),
        ),
        OperatorType::LogicalNotEqual => ConstantNum::new(
            (a.left != b.left) as i8 as f32,
            (a.right != b.right) as i8 as f32,
            a.form.clone(),
        ),
        OperatorType::LogicalGt => ConstantNum::new(
            (a.left > b.left) as i8 as f32,
            (a.right > b.right) as i8 as f32,
            a.form.clone(),
        ),
        OperatorType::LogicalLt => ConstantNum::new(
            (a.left < b.left) as i8 as f32,
            (a.right < b.right) as i8 as f32,
            a.form.clone(),
        ),
        OperatorType::LogicalGte => ConstantNum::new(
            (a.left >= b.left) as i8 as f32,
            (a.right >= b.right) as i8 as f32,
            a.form.clone(),
        ),
        OperatorType::LogicalLte => ConstantNum::new(
            (a.left <= b.left) as i8 as f32,
            (a.right <= b.right) as i8 as f32,
            a.form.clone(),
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
            range.clone(),
        ))
    } else {
        Ok(&tuple.items[index])
    }
}

pub fn const_combine(values: Vec<ConstantValue>) -> ConstantTuple {
    ConstantTuple { items: values }
}

pub fn const_call(function: Function, args: &Iterator<Item = ConstantValue>) -> ConstantValue {
    unimplemented!();
}

// todo: constant function call
