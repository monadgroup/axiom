use ast::{SourcePos, Expression, LValueExpression, OperatorType};

#[derive(Debug)]
pub struct AssignExpression {
    start_pos: SourcePos,
    end_pos: SourcePos,
    left: Box<LValueExpression>,
    right: Box<Expression>,
    operator: OperatorType
}

impl AssignExpression {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, left: Box<LValueExpression>, right: Box<Expression>, operator: OperatorType) -> AssignExpression {
        AssignExpression {
            start_pos,
            end_pos,
            left,
            right,
            operator
        }
    }

    pub fn get_left(&self) -> &Box<LValueExpression> {
        &self.left
    }

    pub fn get_right(&self) -> &Box<Expression> {
        &self.right
    }

    pub fn operator(&self) -> OperatorType {
        self.operator
    }
}

impl Expression for AssignExpression {
    fn get_start_pos(&self) -> SourcePos { self.start_pos }
    fn get_end_pos(&self) -> SourcePos { self.end_pos }
}
