use ast::{ControlField, FormType, OperatorType, UnaryOperation};
use mir::block::Function;
use mir::{ConstantNum, ConstantTuple, ConstantValue};

#[derive(Debug, Clone)]
pub enum Statement {
    Constant(ConstantValue),
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
