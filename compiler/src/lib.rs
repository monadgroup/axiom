extern crate divrem;
extern crate inkwell;
extern crate lazy_static;
extern crate ordered_float;
extern crate regex;

mod compile_error;

pub mod ast;
pub mod codegen;
pub mod frontend;
pub mod mir;
pub mod parser;
pub mod pass;
pub mod util;

pub use crate::compile_error::{CompileError, CompileResult};

// C api
pub use crate::frontend::c_api::*;
