use ast::{SourcePos, Expression};

#[derive(Debug)]
pub struct TupleExpression {
    start_pos: SourcePos,
    end_pos: SourcePos,
    expressions: Vec<Box<Expression>>
}

impl TupleExpression {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, expressions: Vec<Box<Expression>>) -> TupleExpression {
        TupleExpression {
            start_pos,
            end_pos,
            expressions
        }
    }

    pub fn get_expressions(&self) -> &Vec<Box<Expression>> {
        &self.expressions
    }
}

impl Expression for TupleExpression {
    fn get_start_pos(&self) -> SourcePos { self.start_pos }
    fn get_end_pos(&self) -> SourcePos { self.end_pos }
}
