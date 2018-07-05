use mir::VarType;

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Root {
    pub sockets: Vec<VarType>,
}

impl Root {
    pub fn new(sockets: Vec<VarType>) -> Self {
        Root { sockets }
    }
}
