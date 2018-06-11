use std::fmt::Debug;
use ast::SourcePos;

pub trait Expression: Debug {
    fn get_start_pos(&self) -> SourcePos;
    fn get_end_pos(&self) -> SourcePos;
}

pub trait AssignableExpression: Expression {}
