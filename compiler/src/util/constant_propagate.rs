use ast::{FormType, OperatorType, SourceRange, UnaryOperation};
use mir::block::Function;
use mir::{ConstantNum, ConstantTuple, ConstantValue, VarType};
use std::f32::consts;
use {CompileError, CompileResult};

pub fn const_convert(_constant: &ConstantNum, _target_form: &FormType) -> ConstantNum {
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

#[allow(unknown_lints)]
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

fn const_to_num(val: &ConstantValue, range: &SourceRange) -> CompileResult<ConstantNum> {
    match val.as_num() {
        Some(num) => Ok(num.clone()),
        None => Err(CompileError::mismatched_type(
            VarType::Num,
            VarType::of_constant(val),
            *range,
        )),
    }
}

fn consts_to_nums<'a>(
    vals: impl Iterator<Item = &'a ConstantValue>,
    range: &SourceRange,
) -> CompileResult<Vec<ConstantNum>> {
    vals.map(|val| const_to_num(val, range)).collect()
}

fn const_num_intrinsic(
    arg: &ConstantValue,
    range: &SourceRange,
    f: &Fn(f32) -> f32,
) -> CompileResult<ConstantValue> {
    const_to_num(arg, range)
        .and_then(|num| Ok(ConstantValue::new_num(f(num.left), f(num.right), num.form)))
}

fn two_const_num_intrinsic(
    a: &ConstantValue,
    b: &ConstantValue,
    range: &SourceRange,
    f: &Fn(f32, f32) -> f32,
) -> CompileResult<ConstantValue> {
    const_to_num(a, range).and_then(|a_num| {
        const_to_num(b, range).and_then(|b_num| {
            Ok(ConstantValue::new_num(
                f(a_num.left, b_num.left),
                f(a_num.right, b_num.right),
                a_num.form,
            ))
        })
    })
}

pub fn const_call(
    function: &Function,
    args: &[ConstantValue],
    varargs: &[ConstantValue],
    range: &SourceRange,
) -> Option<CompileResult<ConstantValue>> {
    match function {
        Function::Cos => Some(const_num_intrinsic(&args[0], range, &|f| f.cos())),
        Function::Sin => Some(const_num_intrinsic(&args[0], range, &|f| f.sin())),
        Function::Log => Some(const_num_intrinsic(&args[0], range, &|f| f.log(consts::PI))),
        Function::Log2 => Some(const_num_intrinsic(&args[0], range, &|f| f.log2())),
        Function::Log10 => Some(const_num_intrinsic(&args[0], range, &|f| f.log10())),
        Function::Sqrt => Some(const_num_intrinsic(&args[0], range, &|f| f.sqrt())),
        Function::Ceil => Some(const_num_intrinsic(&args[0], range, &|f| f.ceil())),
        Function::Floor => Some(const_num_intrinsic(&args[0], range, &|f| f.floor())),
        Function::Abs => Some(const_num_intrinsic(&args[0], range, &|f| f.abs())),
        Function::Tan => Some(const_num_intrinsic(&args[0], range, &|f| f.tan())),
        Function::Acos => Some(const_num_intrinsic(&args[0], range, &|f| f.acos())),
        Function::Asin => Some(const_num_intrinsic(&args[0], range, &|f| f.asin())),
        Function::Atan => Some(const_num_intrinsic(&args[0], range, &|f| f.atan())),
        Function::Atan2 => Some(two_const_num_intrinsic(
            &args[0],
            &args[1],
            range,
            &|a, b| a.atan2(b),
        )),
        Function::Hypot => Some(two_const_num_intrinsic(
            &args[0],
            &args[1],
            range,
            &|a, b| a.hypot(b),
        )),
        Function::ToRad => Some(const_num_intrinsic(&args[0], range, &|f| f.to_radians())),
        Function::ToDeg => Some(const_num_intrinsic(&args[0], range, &|f| f.to_degrees())),
        Function::Clamp => Some(const_to_num(&args[0], range).and_then(|x| {
            const_to_num(&args[1], range).and_then(|min| {
                const_to_num(&args[2], range).and_then(|max| {
                    Ok(ConstantValue::new_num(
                        x.left.max(min.left).min(max.left),
                        x.right.max(min.right).min(max.right),
                        x.form,
                    ))
                })
            })
        })),
        Function::Pan => {
            Some(const_to_num(&args[0], range).and_then(|val| {
                const_to_num(&args[1], range).and_then(|pan| {
                    // -4.5 dB pan law - see http://www.cs.cmu.edu/~music/icm-online/readings/panlaws/index.html
                    let left_pan = pan.left.max(-1.).min(1.);
                    let left_amt =
                        ((1. - left_pan) * (consts::PI / 4. * (left_pan + 1.)).cos() / 2.).sqrt();

                    let right_pan = pan.right.max(-1.).min(1.);
                    let right_amt =
                        ((1. + right_pan) * (consts::PI / 4. * (right_pan + 1.)).sin() / 2.).sqrt();

                    Ok(ConstantValue::new_num(
                        val.left * left_amt,
                        val.right * right_amt,
                        val.form,
                    ))
                })
            }))
        }
        Function::Left => Some(
            const_to_num(&args[0], range)
                .and_then(|num| Ok(ConstantValue::new_num(num.left, num.left, num.form))),
        ),
        Function::Right => Some(
            const_to_num(&args[0], range)
                .and_then(|num| Ok(ConstantValue::new_num(num.right, num.right, num.form))),
        ),
        Function::Swap => Some(
            const_to_num(&args[0], range)
                .and_then(|num| Ok(ConstantValue::new_num(num.right, num.left, num.form))),
        ),
        Function::Combine => Some(const_to_num(&args[0], range).and_then(|left| {
            const_to_num(&args[1], range)
                .and_then(|right| Ok(ConstantValue::new_num(left.left, right.right, left.form)))
        })),
        Function::Mix => Some(const_to_num(&args[0], range).and_then(|a| {
            const_to_num(&args[1], range).and_then(|b| {
                const_to_num(&args[2], range).and_then(|mix| {
                    Ok(ConstantValue::new_num(
                        a.left + (b.left - a.left) * mix.left,
                        a.right + (b.right - a.right) * mix.right,
                        a.form,
                    ))
                })
            })
        })),
        Function::Sequence => Some(
            consts_to_nums(args.iter().chain(varargs.iter()), range).and_then(|args| {
                let param_count = args.len() - 1;
                let left_index = (args[0].left as usize) % param_count;
                let right_index = (args[0].right as usize) % param_count;

                Ok(ConstantValue::new_num(
                    args[left_index + 1].left,
                    args[right_index + 1].right,
                    args[left_index + 1].form,
                ))
            }),
        ),
        Function::Min => Some(
            consts_to_nums(args.iter().chain(varargs.iter()), range).and_then(|args| {
                let (new_left, new_right) = args.iter()
                    .skip(1)
                    .fold((args[0].left, args[0].right), |(acc_l, acc_r), num| {
                        (acc_l.min(num.left), acc_r.min(num.right))
                    });

                Ok(ConstantValue::new_num(new_left, new_right, args[0].form))
            }),
        ),
        Function::Max => Some(
            consts_to_nums(args.iter().chain(varargs.iter()), range).and_then(|args| {
                let (new_left, new_right) = args.iter()
                    .skip(1)
                    .fold((args[0].left, args[0].right), |(acc_l, acc_r), num| {
                        (acc_l.max(num.left), acc_r.max(num.right))
                    });

                Ok(ConstantValue::new_num(new_left, new_right, args[0].form))
            }),
        ),
        _ => None,
    }
}
