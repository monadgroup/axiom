use ast::{SourceRange, Expression, Form};

#[derive(Debug)]
pub struct NumberExpression {
    pos: SourceRange,
    value: f32,
    form: Form
}

impl NumberExpression {
    pub fn new(pos: SourceRange, value: f32, form: Form) -> NumberExpression {
        NumberExpression {
            pos,
            value,
            form
        }
    }

    pub fn value(&self) -> f32 { self.value }
    pub fn form(&self) -> &Form { &self.form }
}

impl Expression for NumberExpression {
    fn pos(&self) -> &SourceRange { &self.pos }
}
