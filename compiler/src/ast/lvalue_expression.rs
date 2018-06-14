use ast::{Assignable, Expression, SourceRange};

#[derive(Debug)]
pub struct LValueExpression {
    pos: SourceRange,
    assignments: Vec<Assignable>,
}

impl LValueExpression {
    pub fn new(pos: SourceRange, assignments: Vec<Assignable>) -> LValueExpression {
        LValueExpression { pos, assignments }
    }

    pub fn assignments(&self) -> &Vec<Assignable> {
        &self.assignments
    }
}

impl Expression for LValueExpression {
    fn assignables(&self) -> Result<Vec<Assignable>, SourceRange> {
        Ok(self.assignments.clone())
    }
    fn pos(&self) -> &SourceRange {
        &self.pos
    }
}
