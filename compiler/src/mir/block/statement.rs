use crate::ast::{ControlField, FormType, OperatorType, UnaryOperation};
use crate::mir::block::Function;
use crate::mir::{ConstantNum, ConstantTuple, ConstantValue};
use std::fmt;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Global {
    SampleRate,
    BPM,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Statement {
    Constant(ConstantValue),
    Global(Global),
    NumConvert {
        target_form: FormType,
        input: usize,
    },
    NumCast {
        target_form: FormType,
        input: usize,
    },
    NumUnaryOp {
        op: UnaryOperation,
        input: usize,
    },
    NumMathOp {
        op: OperatorType,
        lhs: usize,
        rhs: usize,
    },

    Extract {
        tuple: usize,
        index: usize,
    },
    Combine {
        indexes: Vec<usize>,
    },

    CallFunc {
        function: Function,
        args: Vec<usize>,
        varargs: Vec<usize>,
    },
    StoreControl {
        control: usize,
        field: ControlField,
        value: usize,
    },
    LoadControl {
        control: usize,
        field: ControlField,
    },
}

impl Statement {
    pub fn new_const_num(num: ConstantNum) -> Statement {
        Statement::Constant(ConstantValue::Num(num))
    }

    pub fn new_const_tuple(tuple: ConstantTuple) -> Statement {
        Statement::Constant(ConstantValue::Tuple(tuple))
    }

    pub fn has_side_effect(&self) -> bool {
        match self {
            Statement::Constant(_)
            | Statement::Global(_)
            | Statement::NumConvert { .. }
            | Statement::NumCast { .. }
            | Statement::NumUnaryOp { .. }
            | Statement::NumMathOp { .. }
            | Statement::Extract { .. }
            | Statement::Combine { .. }
            | Statement::LoadControl { .. }
            | Statement::CallFunc { .. } => false,
            Statement::StoreControl { .. } => true,
        }
    }
}

impl fmt::Display for Statement {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Statement::Constant(value) => value.fmt(f),
            Statement::Global(global) => write!(f, "Globals::{:?}", global),
            Statement::NumConvert { target_form, input } => {
                write!(f, "convert %{} to {:?}", input, target_form)
            }
            Statement::NumCast { target_form, input } => {
                write!(f, "cast %{} to {:?}", input, target_form)
            }
            Statement::NumUnaryOp { op, input } => write!(f, "{:?} %{}", op, input),
            Statement::NumMathOp { op, lhs, rhs } => write!(f, "{:?} %{}, %{}", op, lhs, rhs),
            Statement::Extract { tuple, index } => write!(f, "extract {} from %{}", index, tuple),
            Statement::Combine { indexes } => {
                write!(f, "combine ")?;
                for (i, index) in indexes.iter().enumerate() {
                    write!(f, "%{}", index)?;
                    if i != indexes.len() - 1 {
                        write!(f, ", ")?;
                    }
                }
                Ok(())
            }
            Statement::CallFunc {
                function,
                args,
                varargs,
            } => {
                write!(f, "call {:?} (", function)?;
                for (i, index) in args.iter().enumerate() {
                    write!(f, "%{}", index)?;
                    if i != args.len() - 1 {
                        write!(f, ", ")?;
                    }
                }
                if !varargs.is_empty() {
                    write!(f, " ... ")?;
                    for (i, index) in varargs.iter().enumerate() {
                        write!(f, "%{}", index)?;
                        if i != varargs.len() - 1 {
                            write!(f, ", ")?;
                        }
                    }
                }
                write!(f, ")")
            }
            Statement::StoreControl {
                control,
                field,
                value,
            } => write!(f, "store %{} into ${} {}", value, control, field),
            Statement::LoadControl { control, field } => write!(f, "load ${} {}", control, field),
        }
    }
}
