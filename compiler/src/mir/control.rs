use ast::ControlType;

// todo: control needs to store Option<value>

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub struct Control {
    pub control_type: ControlType,
    pub group_id: usize,
    pub value_written: bool,
    pub value_read: bool,
}

impl Control {
    pub fn new(
        control_type: ControlType,
        group_id: usize,
        value_written: bool,
        value_read: bool,
    ) -> Control {
        Control {
            control_type,
            group_id,
            value_written,
            value_read,
        }
    }
}
