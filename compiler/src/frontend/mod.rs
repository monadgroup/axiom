pub mod c_api;
mod dependency_graph;
mod jit;
mod runtime;
pub mod value_reader;

pub use self::dependency_graph::DependencyGraph;
pub use self::jit::Jit;
pub use self::runtime::Runtime;

use crate::mir::{Block, BlockRef, Root, Surface, SurfaceRef};
use std::collections::HashMap;
use std::iter::FromIterator;

#[derive(Debug, Clone)]
pub struct Transaction {
    pub root: Option<Root>,
    pub surfaces: HashMap<SurfaceRef, Surface>,
    pub blocks: HashMap<BlockRef, Block>,
}

impl Transaction {
    pub fn new(root: Option<Root>, surfaces: Vec<Surface>, blocks: Vec<Block>) -> Self {
        Transaction {
            root,
            surfaces: HashMap::from_iter(
                surfaces.into_iter().map(|surface| (surface.id.id, surface)),
            ),
            blocks: HashMap::from_iter(blocks.into_iter().map(|block| (block.id.id, block))),
        }
    }

    pub fn add_surface(&mut self, surface: Surface) {
        self.surfaces.insert(surface.id.id, surface);
    }

    pub fn add_block(&mut self, block: Block) {
        self.blocks.insert(block.id.id, block);
    }
}
