use crate::mir::pool_id::{PoolId, PoolRef};
use crate::mir::SourceMap;
use crate::mir::{Node, ValueGroup};

pub type SurfaceRef = PoolRef;
pub type SurfaceId = PoolId<Surface>;

#[derive(Debug, Clone)]
pub struct Surface {
    pub id: SurfaceId,
    pub groups: Vec<ValueGroup>,
    pub nodes: Vec<Node>,
    pub source_map: SourceMap,
}

impl Surface {
    pub fn new(id: SurfaceId, groups: Vec<ValueGroup>, nodes: Vec<Node>) -> Self {
        Surface {
            id,
            groups,
            nodes,
            source_map: SourceMap::new(),
        }
    }
}
