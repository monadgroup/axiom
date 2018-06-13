use ast::{SourceRange, Expression, OperatorType};

#[derive(Debug)]
pub struct MathExpression {
    pos: SourceRange,
    left: Box<Expression>,
    right: Box<Expression>,
    operator: OperatorType
}

impl MathExpression {
    pub fn new(pos: SourceRange, left: Box<Expression>, right: Box<Expression>, operator: OperatorType) -> MathExpression {
        MathExpression {
            pos,
            left,
            right,
            operator
        }
    }

    pub fn left(&self) -> &Box<Expression> { &self.left }
    pub fn right(&self) -> &Box<Expression> { &self.right }
    pub fn operator(&self) -> OperatorType { self.operator }
}

impl Expression for MathExpression {
    fn pos(&self) -> &SourceRange { &self.pos }
}
