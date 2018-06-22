use mir::{BlockId, SurfaceId, ValueSocket};

#[derive(Debug, PartialEq, Eq, Clone)]
pub enum NodeData {
    Custom(BlockId),
    Group(SurfaceId),
    ExtractGroup {
        surface: SurfaceId,
        source_groups: Vec<usize>,
        dest_groups: Vec<usize>,
    },
}

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Node {
    pub sockets: Vec<ValueSocket>,
    pub data: NodeData,
}

impl Node {
    pub fn new(sockets: Vec<ValueSocket>, data: NodeData) -> Node {
        Node { sockets, data }
    }
}
