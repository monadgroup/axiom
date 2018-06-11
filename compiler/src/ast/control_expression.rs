use ast::{SourcePos, Expression, AssignableExpression, ControlType};

#[derive(Debug)]
pub struct ControlExpression {
    start_pos: SourcePos,
    end_pos: SourcePos,
    name: String,
    control_type: ControlType,
    prop: String
}

impl ControlExpression {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, name: String, control_type: ControlType, prop: String) -> ControlExpression {
        ControlExpression {
            start_pos,
            end_pos,
            name,
            control_type,
            prop
        }
    }

    pub fn get_name(&self) -> &str {
        &self.name
    }

    pub fn get_control_type(&self) -> ControlType {
        self.control_type
    }

    pub fn get_prop(&self) -> &str {
        &self.prop
    }
}

impl Expression for ControlExpression {
    fn get_start_pos(&self) -> SourcePos { self.start_pos }
    fn get_end_pos(&self) -> SourcePos { self.end_pos }
}
impl AssignableExpression for ControlExpression {}
