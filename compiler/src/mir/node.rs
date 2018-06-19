use mir::{BlockId, Control, SurfaceId, ValueSocket};

#[derive(Debug, PartialEq, Eq, Clone)]
pub enum NodeData {
    Custom(BlockId, Vec<Control>),
    Group(SurfaceId),
    ExtractGroup(SurfaceId, Vec<usize>),
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
