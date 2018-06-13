use ast::{SourceRange, Expression};

#[derive(Debug)]
pub struct VariableExpression {
    pos: SourceRange,
    name: String
}

impl VariableExpression {
    pub fn new(pos: SourceRange, name: String) -> VariableExpression {
        VariableExpression {
            pos,
            name
        }
    }

    pub fn get_name(&self) -> &str { &self.name }
}

impl Expression for VariableExpression {
    fn is_assignable(&self) -> bool { true }
    fn pos(&self) -> &SourceRange { &self.pos }
}
