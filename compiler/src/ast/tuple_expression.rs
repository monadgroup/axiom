use ast::{SourceRange, Expression};

#[derive(Debug)]
pub struct TupleExpression {
    pos: SourceRange,
    expressions: Vec<Box<Expression>>
}

impl TupleExpression {
    pub fn new(pos: SourceRange, expressions: Vec<Box<Expression>>) -> TupleExpression {
        TupleExpression {
            pos,
            expressions
        }
    }

    pub fn expressions(&self) -> &Vec<Box<Expression>> { &self.expressions }
}

impl Expression for TupleExpression {
    fn pos(&self) -> &SourceRange { &self.pos }
}
