use crate::ast::FormType;
use ordered_float::OrderedFloat;
use std::cmp::Ordering;
use std::{fmt, hash};

#[derive(Debug, Clone)]
pub struct ConstantNum {
    pub left: f64,
    pub right: f64,
    pub form: FormType,
}

#[derive(Debug, Clone, Eq, PartialEq, Ord, PartialOrd, Hash)]
pub struct ConstantTuple {
    pub items: Vec<ConstantValue>,
}

#[derive(Debug, Clone, Eq, PartialEq, Ord, PartialOrd, Hash)]
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

impl Ord for ConstantNum {
    fn cmp(&self, other: &Self) -> Ordering {
        (OrderedFloat(self.left), OrderedFloat(self.right), self.form).cmp(&(
            OrderedFloat(other.left),
            OrderedFloat(other.right),
            other.form,
        ))
    }
}

impl PartialOrd for ConstantNum {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl fmt::Display for ConstantNum {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "num [{}] {}, {}", self.form, self.left, self.right)
    }
}

impl fmt::Display for ConstantTuple {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "tuple ")?;
        for (i, item) in self.items.iter().enumerate() {
            write!(f, "({})", item)?;

            if i != self.items.len() - 1 {
                write!(f, ", ")?;
            }
        }
        Ok(())
    }
}

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

impl fmt::Display for ConstantValue {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            ConstantValue::Num(num) => num.fmt(f),
            ConstantValue::Tuple(tuple) => tuple.fmt(f),
        }
    }
}
