use ast::{ControlField, FormType, OperatorType, UnaryOperation};
use mir::block::Function;
use std::hash;
use ordered_float::OrderedFloat;

#[derive(Debug, Clone, PartialEq)]
pub struct ConstantNum {
    pub left: f32,
    pub right: f32,
    pub form: FormType,
}

#[derive(Debug, Clone, Eq, PartialEq, Hash)]
pub struct ConstantTuple {
    pub items: Vec<ConstantValue>,
}

#[derive(Debug, Clone, Eq, PartialEq, Hash)]
pub enum ConstantValue {
    Num(ConstantNum),
    Tuple(ConstantTuple),
}

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

impl ConstantNum {
    pub fn new(left: f32, right: f32, form: FormType) -> ConstantNum {
        ConstantNum { left, right, form }
    }

    pub fn with_left(&self, left: f32) -> ConstantNum {
        ConstantNum {
            left,
            right: self.right,
            form: self.form.clone(),
        }
    }

    pub fn with_right(&self, right: f32) -> ConstantNum {
        ConstantNum {
            left: self.left,
            right,
            form: self.form.clone(),
        }
    }

    pub fn with_form(&self, form: FormType) -> ConstantNum {
        ConstantNum {
            left: self.left,
            right: self.right,
            form,
        }
    }
}

impl hash::Hash for ConstantNum {
    fn hash<H: hash::Hasher>(&self, state: &mut H) {
        OrderedFloat(self.left).hash(state);
        OrderedFloat(self.right).hash(state);
        self.form.hash(state);
    }
}

impl Eq for ConstantNum {}

impl ConstantValue {
    pub fn as_num(&self) -> Option<&ConstantNum> {
        if let ConstantValue::Num(ref num) = self {
            Some(num)
        } else {
            None
        }
    }

    pub fn as_tuple(&self) -> Option<&ConstantTuple> {
        if let ConstantValue::Tuple(ref tuple) = self {
            Some(tuple)
        } else {
            None
        }
    }
}

impl Statement {
    pub fn new_const_num(num: ConstantNum) -> Statement {
        Statement::Constant(ConstantValue::Num(num))
    }

    pub fn new_const_tuple(tuple: ConstantTuple) -> Statement {
        Statement::Constant(ConstantValue::Tuple(tuple))
    }
}
