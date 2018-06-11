use ast::{SourcePos, Expression, OperatorType};

#[derive(Debug)]
pub struct MathExpression {
    start_pos: SourcePos,
    end_pos: SourcePos,
    left: Box<Expression>,
    right: Box<Expression>,
    operator: OperatorType
}

impl MathExpression {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, left: Box<Expression>, right: Box<Expression>, operator: OperatorType) -> MathExpression {
        MathExpression {
            start_pos,
            end_pos,
            left,
            right,
            operator
        }
    }

    pub fn get_left(&self) -> &Box<Expression> {
        &self.left
    }

    pub fn get_right(&self) -> &Box<Expression> {
        &self.right
    }

    pub fn get_operator(&self) -> OperatorType {
        self.operator
    }
}

impl Expression for MathExpression {
    fn get_start_pos(&self) -> SourcePos { self.start_pos }
    fn get_end_pos(&self) -> SourcePos { self.end_pos }
}
