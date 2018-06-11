use ast::{SourcePos, Expression};

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum UnaryOperation {
    Positive,
    Negative,
    Not
}

#[derive(Debug)]
pub struct UnaryExpression {
    start_pos: SourcePos,
    end_pos: SourcePos,
    operation: UnaryOperation,
    expr: Box<Expression>
}

impl UnaryExpression {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, operation: UnaryOperation, expr: Box<Expression>) -> UnaryExpression {
        UnaryExpression {
            start_pos,
            end_pos,
            operation,
            expr
        }
    }

    pub fn get_operation(&self) -> UnaryOperation {
        self.operation
    }

    pub fn get_expr(&self) -> &Box<Expression> {
        &self.expr
    }
}

impl Expression for UnaryExpression {
    fn get_start_pos(&self) -> SourcePos { self.start_pos }
    fn get_end_pos(&self) -> SourcePos { self.end_pos }
}
