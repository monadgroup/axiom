use ast::{Assignable, Expression, SourceRange};

#[derive(Debug)]
pub struct TupleExpression {
    pos: SourceRange,
    expressions: Vec<Box<Expression>>,
}

impl TupleExpression {
    pub fn new(pos: SourceRange, expressions: Vec<Box<Expression>>) -> TupleExpression {
        TupleExpression { pos, expressions }
    }

    pub fn expressions(&self) -> &Vec<Box<Expression>> {
        &self.expressions
    }
}

impl Expression for TupleExpression {
    fn assignables(&self) -> Result<Vec<Assignable>, SourceRange> {
        let mut result = Vec::new();
        for expr in &self.expressions {
            match expr.assignables() {
                Ok(assignables) => result.extend(assignables),
                Err(err) => return Err(err),
            }
        }
        Ok(result)
    }
    fn pos(&self) -> &SourceRange {
        &self.pos
    }
}
