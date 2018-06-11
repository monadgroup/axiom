use ast::{SourcePos, Expression, LValueExpression};

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum PostfixOperation {
    Increment,
    Decrement
}

#[derive(Debug)]
pub struct PostfixExpression {
    start_pos: SourcePos,
    end_pos: SourcePos,
    left: Box<LValueExpression>,
    operation: PostfixOperation
}

impl PostfixExpression {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, left: Box<LValueExpression>, operation: PostfixOperation) -> PostfixExpression {
        PostfixExpression {
            start_pos,
            end_pos,
            left,
            operation
        }
    }

    pub fn get_left(&self) -> &Box<LValueExpression> {
        &self.left
    }

    pub fn get_operation(&self) -> PostfixOperation {
        self.operation
    }
}

impl Expression for PostfixExpression {
    fn get_start_pos(&self) -> SourcePos { self.start_pos }
    fn get_end_pos(&self) -> SourcePos { self.end_pos }
}
