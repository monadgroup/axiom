use ast::{Assignable, Expression, SourceRange};

#[derive(Debug)]
pub struct VariableExpression {
    pos: SourceRange,
    name: String,
}

impl VariableExpression {
    pub fn new(pos: SourceRange, name: String) -> VariableExpression {
        VariableExpression { pos, name }
    }

    pub fn get_name(&self) -> &str {
        &self.name
    }
}

impl Expression for VariableExpression {
    fn assignables(&self) -> Result<Vec<Assignable>, SourceRange> {
        Ok(vec![Assignable::Variable {
            pos: self.pos,
            name: self.name.to_owned(),
        }])
    }
    fn pos(&self) -> &SourceRange {
        &self.pos
    }
}
