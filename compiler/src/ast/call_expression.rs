use ast::{SourcePos, Expression};

#[derive(Debug)]
pub struct CallExpression {
    start_pos: SourcePos,
    end_pos: SourcePos,
    name: String,
    arguments: Vec<Box<Expression>>
}

impl CallExpression {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, name: String, arguments: Vec<Box<Expression>>) -> CallExpression {
        CallExpression {
            start_pos,
            end_pos,
            name,
            arguments
        }
    }

    pub fn get_name(&self) -> &str {
        &self.name
    }

    pub fn get_arguments(&self) -> &Vec<Box<Expression>> {
        &self.arguments
    }
}

impl Expression for CallExpression {
    fn get_start_pos(&self) -> SourcePos { self.start_pos }
    fn get_end_pos(&self) -> SourcePos { self.end_pos }
}
