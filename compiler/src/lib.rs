extern crate regex;

#[macro_use]
extern crate lazy_static;

mod ast;
mod compile_error;
mod mir;
mod parser;

pub use self::ast::*;
pub use self::compile_error::{CompileError, CompileResult};
pub use self::mir::*;
pub use self::parser::*;
