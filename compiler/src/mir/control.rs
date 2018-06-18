use ast::ControlType;

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Control {
    pub control_type: ControlType,
}

impl Control {
    pub fn new(control_type: ControlType) -> Control {
        Control { control_type }
    }
}
