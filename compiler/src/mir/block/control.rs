use ast::ControlType;

#[derive(Debug, Clone)]
pub struct Control {
    pub name: String,
    pub control_type: ControlType,
    pub value_written: bool,
    pub value_read: bool,
}

impl Control {
    pub fn new(
        name: String,
        control_type: ControlType,
        value_written: bool,
        value_read: bool,
    ) -> Control {
        Control {
            name,
            control_type,
            value_written,
            value_read,
        }
    }
}
