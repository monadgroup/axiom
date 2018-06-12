use std::error::Error;
use std::fmt;
use ast::SourceRange;
use parser::{TokenType, Token};

pub enum CompileError {
    UnexpectedToken { expected: TokenType, found: Token }
}

impl CompileError {
    pub fn unexpected_token(expected: TokenType, found: Token) -> CompileError {
        CompileError::UnexpectedToken {
            expected,
            found
        }
    }
}
