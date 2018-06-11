use ast::{SourcePos, Expression, Form};

#[derive(Debug)]
pub struct NumberExpression {
    start_pos: SourcePos,
    end_pos: SourcePos,
    value: f32,
    form: Form
}

impl NumberExpression {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, value: f32, form: Form) -> NumberExpression {
        NumberExpression {
            start_pos,
            end_pos,
            value,
            form
        }
    }

    pub fn get_value(&self) -> f32 {
        self.value
    }

    pub fn get_form(&self) -> &Form {
        &self.form
    }
}

impl Expression for NumberExpression {
    fn get_start_pos(&self) -> SourcePos { self.start_pos }
    fn get_end_pos(&self) -> SourcePos { self.end_pos }
}
