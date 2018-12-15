use crate::mir::{ConstantValue, VarType};

#[derive(Debug, PartialEq, Eq, Clone, Hash, PartialOrd, Ord)]
pub enum ValueGroupSource {
    None,
    Socket(usize),
    Default(ConstantValue),
}

#[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Clone, Hash)]
pub struct ValueGroup {
    pub value_type: VarType,
    pub source: ValueGroupSource,
}

impl ValueGroup {
    pub fn new(value_type: VarType, source: ValueGroupSource) -> Self {
        ValueGroup { value_type, source }
    }
}
