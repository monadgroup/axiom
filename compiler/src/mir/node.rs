use crate::mir::{BlockRef, ControlInitializer, SurfaceRef, ValueSocket};
use std::fmt;

#[derive(Debug, PartialEq, Clone, Hash)]
pub enum NodeData {
    Dummy,
    Custom {
        block: BlockRef,
        control_initializers: Vec<ControlInitializer>,
    },
    Group(SurfaceRef),
    ExtractGroup {
        surface: SurfaceRef,
        source_sockets: Vec<usize>,
        dest_sockets: Vec<usize>,
    },
}

#[derive(Debug, PartialEq, Clone, Hash)]
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
            NodeData::Custom {
                block,
                control_initializers,
            } => {
                write!(f, "block @{} -> ", block)?;
                for (i, initializer) in control_initializers.iter().enumerate() {
                    write!(f, "{}", initializer)?;
                    if i != control_initializers.len() - 1 {
                        write!(f, ", ")?;
                    }
                }
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
