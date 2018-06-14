use mir::{Block, BlockId, Surface, SurfaceId};
use std::collections::HashMap;

#[derive(Debug, Clone)]
pub struct MIRContext {
    next_id: u64,
    surfaces: HashMap<SurfaceId, Surface>,
    blocks: HashMap<BlockId, Block>,
}

impl MIRContext {
    pub fn alloc_id(&mut self) -> u64 {
        let new_id = self.next_id;
        self.next_id += 1;
        new_id
    }

    pub fn get_surface(&self, id: &SurfaceId) -> Option<&Surface> {
        self.surfaces.get(id)
    }

    pub fn get_surface_mut(&mut self, id: &SurfaceId) -> Option<&mut Surface> {
        self.surfaces.get_mut(id)
    }

    pub fn get_block(&self, id: &BlockId) -> Option<&Block> {
        self.blocks.get(id)
    }

    pub fn get_block_mut(&mut self, id: &BlockId) -> Option<&mut Block> {
        self.blocks.get_mut(id)
    }
}
