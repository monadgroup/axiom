use ast::Expression;

#[derive(Debug)]
pub struct Block {
    expressions: Vec<Box<Expression>>
}

impl Block {
    pub fn new(expressions: Vec<Box<Expression>>) -> Block {
        Block {
            expressions
        }
    }

    pub fn expressions(&self) -> &Vec<Box<Expression>> { &self.expressions }
}
