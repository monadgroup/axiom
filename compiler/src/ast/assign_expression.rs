use ast::{Expression, LValueExpression, OperatorType, SourceRange};

#[derive(Debug)]
pub struct AssignExpression {
    pos: SourceRange,
    left: LValueExpression,
    right: Box<Expression>,
    operator: OperatorType,
}

impl AssignExpression {
    pub fn new(
        pos: SourceRange,
        left: LValueExpression,
        right: Box<Expression>,
        operator: OperatorType,
    ) -> AssignExpression {
        AssignExpression {
            pos,
            left,
            right,
            operator,
        }
    }

    pub fn left(&self) -> &LValueExpression {
        &self.left
    }
    pub fn right(&self) -> &Expression {
        self.right.as_ref()
    }
    pub fn operator(&self) -> OperatorType {
        self.operator
    }
}

impl Expression for AssignExpression {
    fn pos(&self) -> &SourceRange {
        &self.pos
    }
}
