use ast::{SourceRange, Expression};

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum UnaryOperation {
    Positive,
    Negative,
    Not
}

#[derive(Debug)]
pub struct UnaryExpression {
    pos: SourceRange,
    operation: UnaryOperation,
    expr: Box<Expression>
}

impl UnaryExpression {
    pub fn new(pos: SourceRange, operation: UnaryOperation, expr: Box<Expression>) -> UnaryExpression {
        UnaryExpression {
            pos,
            operation,
            expr
        }
    }

    pub fn operation(&self) -> UnaryOperation { self.operation }
    pub fn expr(&self) -> &Box<Expression> { &self.expr }
}

impl Expression for UnaryExpression {
    fn pos(&self) -> &SourceRange { &self.pos }
}
