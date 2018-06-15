extern crate ordered_float;
extern crate regex;

#[macro_use]
extern crate lazy_static;

mod compile_error;

pub mod ast;
pub mod mir;
pub mod parser;
pub mod pass;
pub mod util;

pub use compile_error::{CompileError, CompileResult};
