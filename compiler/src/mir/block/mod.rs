use mir::pool_id::PoolId;

mod control;
mod statement;
mod function;

pub use self::control::Control;
pub use self::statement::Statement;
pub use self::function::Function;

pub type BlockId = PoolId<Block>;

#[derive(Debug, Clone)]
pub struct Block {
    pub id: BlockId,
    pub controls: Vec<Control>,
    pub statements: Vec<Statement>
}

impl Block {
    pub fn new(id: BlockId, controls: Vec<Control>, statements: Vec<Statement>) -> Block {
        Block {
            id,
            controls,
            statements
        }
    }
}
