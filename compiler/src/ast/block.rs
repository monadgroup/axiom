use ast::Expression;

#[derive(Debug)]
pub struct Block {
    pub expressions: Vec<Expression>,
}

impl Block {
    pub fn new(expressions: Vec<Expression>) -> Block {
        Block { expressions }
    }
}
