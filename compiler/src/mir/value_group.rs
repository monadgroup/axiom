use mir::{ConstantValue, VarType};

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct ValueGroup {
    pub value_type: VarType,
    pub exposer: Option<usize>,
    pub default_val: Option<ConstantValue>,
}

impl ValueGroup {
    pub fn new(
        value_type: VarType,
        exposer: Option<usize>,
        default_val: Option<ConstantValue>,
    ) -> Self {
        ValueGroup {
            value_type,
            exposer,
            default_val,
        }
    }
}
