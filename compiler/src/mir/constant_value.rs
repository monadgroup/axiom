use crate::ast::FormType;
use ordered_float::OrderedFloat;
use std::hash;

#[derive(Debug, Clone)]
pub struct ConstantNum {
    pub left: f64,
    pub right: f64,
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

impl ConstantNum {
    pub fn new(left: f64, right: f64, form: FormType) -> ConstantNum {
        ConstantNum { left, right, form }
    }

    pub fn with_left(&self, left: f64) -> ConstantNum {
        ConstantNum {
            left,
            right: self.right,
            form: self.form,
        }
    }

    pub fn with_right(&self, right: f64) -> ConstantNum {
        ConstantNum {
            left: self.left,
            right,
            form: self.form,
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

impl PartialEq for ConstantNum {
    fn eq(&self, other: &ConstantNum) -> bool {
        OrderedFloat(self.left).eq(&OrderedFloat(other.left))
            && OrderedFloat(self.right).eq(&OrderedFloat(other.right))
            && self.form.eq(&other.form)
    }
}

impl Eq for ConstantNum {}

impl ConstantValue {
    pub fn new_num(left: f64, right: f64, form: FormType) -> ConstantValue {
        ConstantValue::Num(ConstantNum::new(left, right, form))
    }

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
