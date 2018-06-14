use ast::{Expression, Form, SourceRange};

#[derive(Debug)]
pub struct CastExpression {
    pos: SourceRange,
    target: Form,
    expr: Box<Expression>,
    is_convert: bool,
}

impl CastExpression {
    pub fn new(
        pos: SourceRange,
        target: Form,
        expr: Box<Expression>,
        is_convert: bool,
    ) -> CastExpression {
        CastExpression {
            pos,
            target,
            expr,
            is_convert,
        }
    }

    pub fn target(&self) -> &Form {
        &self.target
    }
    pub fn expr(&self) -> &Expression {
        self.expr.as_ref()
    }
    pub fn is_convert(&self) -> bool {
        self.is_convert
    }
}

impl Expression for CastExpression {
    fn pos(&self) -> &SourceRange {
        &self.pos
    }
}
