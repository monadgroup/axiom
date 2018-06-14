use std::io;
use std::io::prelude::*;

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

fn run_code(code: &str) {
    let mut stream = get_token_stream(code);
    match Parser::parse(&mut stream) {
        Ok(ast) => println!("AST: {:#?}", ast),
        Err(err) => {
            let (text, pos) = err.formatted();
            println!(
                "Error {}:{} to {}:{} - {}",
                pos.0.line, pos.0.column, pos.1.line, pos.1.column, text
            );
        }
    }
}

fn do_repl() {
    println!("Enter code followed by two newlines...");
    let stdin = io::stdin();
    let mut input = "".to_owned();
    for line in stdin.lock().lines() {
        let unwrapped = line.unwrap();
        input.push_str(&unwrapped);
        input.push_str("\n");

        if unwrapped.is_empty() {
            run_code(&input);
            return;
        }
    }
}

fn main() {
    loop {
        do_repl();
    }
}
