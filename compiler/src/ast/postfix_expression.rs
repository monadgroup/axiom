use ast::{Expression, LValueExpression, SourceRange};

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum PostfixOperation {
    Increment,
    Decrement,
}

#[derive(Debug)]
pub struct PostfixExpression {
    pos: SourceRange,
    left: LValueExpression,
    operation: PostfixOperation,
}

impl PostfixExpression {
    pub fn new(
        pos: SourceRange,
        left: LValueExpression,
        operation: PostfixOperation,
    ) -> PostfixExpression {
        PostfixExpression {
            pos,
            left,
            operation,
        }
    }

    pub fn left(&self) -> &LValueExpression {
        &self.left
    }
    pub fn operation(&self) -> PostfixOperation {
        self.operation
    }
}

impl Expression for PostfixExpression {
    fn pos(&self) -> &SourceRange {
        &self.pos
    }
}
