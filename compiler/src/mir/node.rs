use mir::{BlockId, Control, SurfaceId};

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Node {
    pub controls: Vec<Control>,
    pub block_id: Option<BlockId>,
    pub surface_id: Option<SurfaceId>,
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
}
