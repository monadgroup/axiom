use ast::{Expression, SourceRange};

#[derive(Debug)]
pub struct CallExpression {
    pos: SourceRange,
    name: String,
    arguments: Vec<Box<Expression>>,
}

impl CallExpression {
    pub fn new(pos: SourceRange, name: String, arguments: Vec<Box<Expression>>) -> CallExpression {
        CallExpression {
            pos,
            name,
            arguments,
        }
    }

    pub fn name(&self) -> &str {
        &self.name
    }
    pub fn arguments(&self) -> &Vec<Box<Expression>> {
        &self.arguments
    }
}

impl Expression for CallExpression {
    fn pos(&self) -> &SourceRange {
        &self.pos
    }
}
