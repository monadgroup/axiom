pub mod mir_builder;

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
