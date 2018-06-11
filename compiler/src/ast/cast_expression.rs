use ast::{SourcePos, Expression, Form};

#[derive(Debug)]
pub struct CastExpression {
    start_pos: SourcePos,
    end_pos: SourcePos,
    target: Form,
    expr: Box<Expression>,
    is_convert: bool
}

impl CastExpression {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, target: Form, expr: Box<Expression>, is_convert: bool) -> CastExpression {
        CastExpression {
            start_pos,
            end_pos,
            target,
            expr,
            is_convert
        }
    }

    pub fn get_target(&self) -> &Form {
        &self.target
    }

    pub fn get_expr(&self) -> &Box<Expression> {
        &self.expr
    }

    pub fn get_is_convert(&self) -> bool {
        self.is_convert
    }
}

impl Expression for CastExpression {
    fn get_start_pos(&self) -> SourcePos { self.start_pos }
    fn get_end_pos(&self) -> SourcePos { self.end_pos }
}
