use std::fmt::Debug;
use ast::{SourceRange, ControlType};

#[derive(Debug, Clone)]
pub enum Assignable {
    Control { pos: SourceRange, name: String, control_type: ControlType, prop: String },
    Variable { pos: SourceRange, name: String }
}

pub trait Expression: Debug {
    fn assignables(&self) -> Result<Vec<Assignable>, SourceRange> { Err(self.pos().clone()) }
    fn pos(&self) -> &SourceRange;
}
