extern crate regex;

#[macro_use]
extern crate lazy_static;

mod ast;
mod mir;
mod parser;

pub use self::ast::*;
pub use self::mir::*;
pub use self::parser::*;
