extern crate regex;
extern crate ordered_float;

#[macro_use]
extern crate lazy_static;

mod compile_error;

pub mod ast;
pub mod mir;
pub mod parser;
pub mod pass;
pub mod util;

pub use compile_error::{CompileError, CompileResult};
