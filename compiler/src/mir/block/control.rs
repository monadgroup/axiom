use ast::ControlType;

#[derive(Debug, Clone)]
pub struct Control {
    pub name: String,
    pub control_type: ControlType,
    pub value_written: bool,
    pub value_read: bool,
}
