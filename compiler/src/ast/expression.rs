use ast::{ControlType, SourceRange};
use std::fmt::Debug;

#[derive(Debug, Clone)]
pub enum Assignable {
    Control {
        pos: SourceRange,
        name: String,
        control_type: ControlType,
        prop: String,
    },
    Variable {
        pos: SourceRange,
        name: String,
    },
}

pub trait Expression: Debug {
    fn assignables(&self) -> Result<Vec<Assignable>, SourceRange> {
        Err(*self.pos())
    }
    fn pos(&self) -> &SourceRange;
}
