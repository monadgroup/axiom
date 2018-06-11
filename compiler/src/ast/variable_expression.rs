use ast::{SourcePos, Expression, AssignableExpression};

#[derive(Debug)]
pub struct VariableExpression {
    start_pos: SourcePos,
    end_pos: SourcePos,
    name: String
}

impl VariableExpression {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, name: String) -> VariableExpression {
        VariableExpression {
            start_pos,
            end_pos,
            name
        }
    }

    pub fn get_name(&self) -> &str {
        &self.name
    }
}

impl Expression for VariableExpression {
    fn get_start_pos(&self) -> SourcePos { self.start_pos }
    fn get_end_pos(&self) -> SourcePos { self.end_pos }
}
impl AssignableExpression for VariableExpression {}
