use mir::pool_id::PoolId;
use mir::{ControlGroup, Node};

pub type SurfaceId = PoolId<Surface>;

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Surface {
    pub id: SurfaceId,
    pub groups: Vec<ControlGroup>,
    pub nodes: Vec<Node>,
}

impl Surface {
    pub fn new(id: SurfaceId, groups: Vec<ControlGroup>, nodes: Vec<Node>) -> Self {
        Surface { id, groups, nodes }
    }
}
