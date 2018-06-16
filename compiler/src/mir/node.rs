use mir::{BlockId, Control, SurfaceId};

#[derive(Debug, PartialEq, Eq, Clone)]
pub enum NodeData {
    Custom(BlockId),
    Group(SurfaceId),
    ExtractGroup(SurfaceId, Vec<usize>),
}

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Node {
    pub controls: Vec<Control>,
    pub data: NodeData,
}

impl Node {
    pub fn new(controls: Vec<Control>, data: NodeData) -> Node {
        Node { controls, data }
    }
}
