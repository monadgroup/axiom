use mir::{BlockId, Control, SurfaceId};

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Node {
    controls: Vec<Control>,
    block_id: Option<BlockId>,
    surface_id: Option<SurfaceId>,
}

impl Node {
    pub fn new(
        controls: Vec<Control>,
        block_id: Option<BlockId>,
        surface_id: Option<SurfaceId>,
    ) -> Node {
        Node {
            controls,
            block_id,
            surface_id,
        }
    }

    pub fn get_controls(&self) -> &Vec<Control> {
        &self.controls
    }

    pub fn get_block_id(&self) -> &Option<BlockId> {
        &self.block_id
    }

    pub fn get_surface_id(&self) -> &Option<SurfaceId> {
        &self.surface_id
    }
}
