pub mod dependency_walker;
pub mod mir_builder;
mod runtime;

pub use self::runtime::Runtime;

use mir::{Block, Surface};

#[derive(Debug, Clone)]
pub struct Transaction {
    pub surfaces: Vec<Surface>,
    pub blocks: Vec<Block>,
}

impl Transaction {
    pub fn new(surfaces: Vec<Surface>, blocks: Vec<Block>) -> Self {
        Transaction { surfaces, blocks }
    }
}
