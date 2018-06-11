use ast::{SourcePos, Expression, AssignableExpression};

#[derive(Debug)]
pub struct LValueExpression {
    start_pos: SourcePos,
    end_pos: SourcePos,
    assignments: Vec<Box<AssignableExpression>>
}

impl LValueExpression {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, assignments: Vec<Box<AssignableExpression>>) -> LValueExpression {
        LValueExpression {
            start_pos,
            end_pos,
            assignments
        }
    }

    pub fn get_assignments(&self) -> &Vec<Box<AssignableExpression>> {
        &self.assignments
    }
}

impl Expression for LValueExpression {
    fn get_start_pos(&self) -> SourcePos { self.start_pos }
    fn get_end_pos(&self) -> SourcePos { self.end_pos }
}
