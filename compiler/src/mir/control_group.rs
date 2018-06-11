use mir::VarType;

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct ControlGroup {
    value_type: VarType,
    extracted: bool,
    value_written: bool,
    value_read: bool,
}

impl ControlGroup {
    pub fn new(value_type: VarType, extracted: bool, value_written: bool, value_read: bool) -> Self {
        ControlGroup {
            value_type,
            extracted,
            value_written,
            value_read
        }
    }

    pub fn get_value_type(&self) -> VarType {
        self.value_type
    }

    pub fn get_extracted(&self) -> bool {
        self.extracted
    }

    pub fn get_value_written(&self) -> bool {
        self.value_written
    }

    pub fn get_value_read(&self) -> bool {
        self.value_read
    }
}
