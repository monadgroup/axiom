use ast::{SourceRange, Expression};

#[derive(Debug)]
pub struct LValueExpression {
    pos: SourceRange,
    assignments: Vec<Box<Expression>>
}

impl LValueExpression {
    pub fn new(pos: SourceRange, assignments: Vec<Box<Expression>>) -> LValueExpression {
        LValueExpression {
            pos,
            assignments
        }
    }

    pub fn assignments(&self) -> &Vec<Box<Expression>> { &self.assignments }
}

impl Expression for LValueExpression {
    fn is_assignable(&self) -> bool { true }
    fn pos(&self) -> &SourceRange { &self.pos }
}
