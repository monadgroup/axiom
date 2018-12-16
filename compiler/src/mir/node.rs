use crate::mir::{BlockRef, SurfaceRef, ValueSocket};
use std::fmt;

#[derive(Debug, PartialEq, Eq, Clone, Hash)]
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

#[derive(Debug, PartialEq, Eq, Clone, Hash)]
pub struct Node {
    pub sockets: Vec<ValueSocket>,
    pub data: NodeData,
}

impl Node {
    pub fn new(sockets: Vec<ValueSocket>, data: NodeData) -> Node {
        Node { sockets, data }
    }
}

impl fmt::Display for Node {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let source_dest_indices = match &self.data {
            NodeData::Dummy => {
                write!(f, "dummy")?;
                None
            }
            NodeData::Custom(block) => {
                write!(f, "block @{}", block)?;
                None
            }
            NodeData::Group(surface) => {
                write!(f, "group @{}", surface)?;
                None
            }
            NodeData::ExtractGroup {
                surface,
                source_sockets,
                dest_sockets,
            } => {
                write!(f, "extract @{}", surface)?;
                Some((source_sockets, dest_sockets))
            }
        };

        write!(f, " (")?;
        for (socket_index, socket) in self.sockets.iter().enumerate() {
            write!(f, "{}", socket)?;

            if let Some((source_indices, dest_indices)) = source_dest_indices {
                if source_indices.contains(&socket_index) {
                    write!(f, " {{source}}")?;
                }
                if dest_indices.contains(&socket_index) {
                    write!(f, " {{dest}}")?;
                }
            }

            if socket_index != self.sockets.len() - 1 {
                write!(f, ", ")?;
            }
        }

        write!(f, ")")
    }
}
