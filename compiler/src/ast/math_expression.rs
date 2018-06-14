use ast::{Expression, OperatorType, SourceRange};

#[derive(Debug)]
pub struct MathExpression {
    pos: SourceRange,
    left: Box<Expression>,
    right: Box<Expression>,
    operator: OperatorType,
}

impl MathExpression {
    pub fn new(
        pos: SourceRange,
        left: Box<Expression>,
        right: Box<Expression>,
        operator: OperatorType,
    ) -> MathExpression {
        MathExpression {
            pos,
            left,
            right,
            operator,
        }
    }

    pub fn left(&self) -> &Expression {
        self.left.as_ref()
    }
    pub fn right(&self) -> &Expression {
        self.right.as_ref()
    }
    pub fn operator(&self) -> OperatorType {
        self.operator
    }
}

impl Expression for MathExpression {
    fn pos(&self) -> &SourceRange {
        &self.pos
    }
}
