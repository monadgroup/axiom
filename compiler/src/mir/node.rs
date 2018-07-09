use mir::{BlockRef, SurfaceRef, ValueSocket};

#[derive(Debug, PartialEq, Eq, Clone)]
pub enum NodeData {
    Dummy,
    Custom(BlockRef),
    Group(SurfaceRef),
    ExtractGroup {
        surface: SurfaceRef,
        source_sockets: Vec<usize>,
        dest_sockets: Vec<usize>,
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
