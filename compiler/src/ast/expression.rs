use std::fmt::Debug;
use ast::SourceRange;

pub trait Expression: Debug {
    fn is_assignable(&self) -> bool { false }
    fn pos(&self) -> &SourceRange;
}
