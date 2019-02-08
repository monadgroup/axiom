use crate::mir::pool_id::{PoolId, PoolRef};
use crate::mir::VarType;
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
        writeln!(f, "block @{:?} {{", self.id)?;
        writeln!(f, "  controls:")?;
        for (i, control) in self.controls.iter().enumerate() {
            writeln!(f, "    ${} = {}", i, control)?;
        }
        writeln!(f, "  statements:")?;
        for (i, statement) in self.statements.iter().enumerate() {
            writeln!(
                f,
                "    %{} {} = {}",
                i,
                VarType::of_statement(self, i),
                statement
            )?;
        }
        write!(f, "}}")
    }
}
