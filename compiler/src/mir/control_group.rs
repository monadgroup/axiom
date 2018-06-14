use mir::VarType;

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct ControlGroup {
    pub value_type: VarType,
    pub extracted: bool,
    pub value_written: bool,
    pub value_read: bool,
}

impl ControlGroup {
    pub fn new(
        value_type: VarType,
        extracted: bool,
        value_written: bool,
        value_read: bool,
    ) -> Self {
        ControlGroup {
            value_type,
            extracted,
            value_written,
            value_read,
        }
    }
}
