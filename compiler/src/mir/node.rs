use mir::{BlockId, Control, SurfaceId, ValueSocket};

#[derive(Debug, PartialEq, Eq, Clone)]
pub enum NodeData {
    Custom(BlockId),
    Group(SurfaceId),
    ExtractGroup(SurfaceId, Vec<usize>),
}

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Node {
    pub sockets: Vec<ValueSocket>,
    pub controls: Vec<Control>,
    pub data: NodeData,
}

impl Node {
    pub fn new(sockets: Vec<ValueSocket>, controls: Vec<Control>, data: NodeData) -> Node {
        Node {
            sockets,
            controls,
            data,
        }
    }
}
