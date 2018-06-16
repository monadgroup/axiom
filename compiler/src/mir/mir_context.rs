use mir::{Block, BlockId, Surface, SurfaceId};
use std::collections::HashMap;

pub struct Transaction {
    pub surfaces: Vec<Surface>,
    pub blocks: Vec<Block>,
}

#[derive(Debug, Clone)]
pub struct MIRContext {
    next_id: u64,
    surfaces: HashMap<SurfaceId, Surface>,
    blocks: HashMap<BlockId, Block>,
}

impl Transaction {
    pub fn new(surfaces: Vec<Surface>, blocks: Vec<Block>) -> Self {
        Transaction { surfaces, blocks }
    }
}

impl MIRContext {
    pub fn alloc_id(&mut self) -> u64 {
        let new_id = self.next_id;
        self.next_id += 1;
        new_id
    }

    pub fn surface(&self, id: &SurfaceId) -> Option<&Surface> {
        self.surfaces.get(id)
    }

    pub fn surface_mut(&mut self, id: &SurfaceId) -> Option<&mut Surface> {
        self.surfaces.get_mut(id)
    }

    pub fn block(&self, id: &BlockId) -> Option<&Block> {
        self.blocks.get(id)
    }

    pub fn block_mut(&mut self, id: &BlockId) -> Option<&mut Block> {
        self.blocks.get_mut(id)
    }

    /*pub fn commit(&mut self, transaction: Transaction) {
        unimplemented!();
    }*/
}
