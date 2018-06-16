use mir::{ConstantValue, VarType};

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct ControlGroup {
    pub value_type: VarType,
    pub value_written: bool,
    pub value_read: bool,
    pub default_val: Option<ConstantValue>,
}

impl ControlGroup {
    pub fn new(
        value_type: VarType,
        value_written: bool,
        value_read: bool,
        default_val: Option<ConstantValue>,
    ) -> Self {
        ControlGroup {
            value_type,
            value_written,
            value_read,
            default_val,
        }
    }
}
