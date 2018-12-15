use crate::mir::pool_id::{PoolId, PoolRef};
use std::fmt;

mod control;
mod function;
mod statement;

pub use self::control::Control;
pub use self::function::{Function, FunctionArgRange, FUNCTION_TABLE};
pub use self::statement::{Global, Statement};

pub type BlockRef = PoolRef;
pub type BlockId = PoolId<Block>;

#[derive(Debug, Clone)]
pub struct Block {
    pub id: BlockId,
    pub controls: Vec<Control>,
    pub statements: Vec<Statement>,
}

impl Block {
    pub fn new(id: BlockId, controls: Vec<Control>, statements: Vec<Statement>) -> Block {
        Block {
            id,
            controls,
            statements,
        }
    }
}

impl fmt::Display for Block {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "block @{:?} {{\n", self.id)?;
        write!(f, "  controls:\n")?;
        for (i, control) in self.controls.iter().enumerate() {
            write!(f, "    ${} = {}\n", i, control)?;
        }
        write!(f, "  statements:\n")?;
        for (i, statement) in self.statements.iter().enumerate() {
            write!(f, "    %{} = {}\n", i, statement)?;
        }
        write!(f, "}}")
    }
}
