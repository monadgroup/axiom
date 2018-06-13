use ast::{SourceRange, Expression, LValueExpression, OperatorType};

#[derive(Debug)]
pub struct AssignExpression {
    pos: SourceRange,
    left: Box<LValueExpression>,
    right: Box<Expression>,
    operator: OperatorType
}

impl AssignExpression {
    pub fn new(pos: SourceRange, left: Box<LValueExpression>, right: Box<Expression>, operator: OperatorType) -> AssignExpression {
        AssignExpression {
            pos,
            left,
            right,
            operator
        }
    }

    pub fn left(&self) -> &Box<LValueExpression> { &self.left }
    pub fn right(&self) -> &Box<Expression> { &self.right }
    pub fn operator(&self) -> OperatorType { self.operator }
}

impl Expression for AssignExpression {
    fn pos(&self) -> &SourceRange { &self.pos }
}
