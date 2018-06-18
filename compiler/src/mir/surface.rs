use mir::pool_id::PoolId;
use mir::{Node, ValueGroup};

pub type SurfaceId = PoolId<Surface>;

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Surface {
    pub id: SurfaceId,
    pub groups: Vec<ValueGroup>,
    pub nodes: Vec<Node>,
}

impl Surface {
    pub fn new(id: SurfaceId, groups: Vec<ValueGroup>, nodes: Vec<Node>) -> Self {
        Surface { id, groups, nodes }
    }
}
