use ast::Expression;

#[derive(Debug)]
pub struct Block {
    expressions: Vec<Expression>,
}

impl Block {
    pub fn new(expressions: Vec<Expression>) -> Block {
        Block { expressions }
    }

    pub fn expressions(&self) -> &Vec<Expression> {
        &self.expressions
    }
}
