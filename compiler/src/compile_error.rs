use std::error::Error;
use std::fmt;
use ast::SourceRange;
use parser::{TokenType, Token};

pub enum CompileError {
    MismatchedToken { expected: TokenType, found: Token },
    UnexpectedToken(Token),
    UnexpectedEnd,
    UnknownForm(String, SourceRange),
    UnknownNote(String, SourceRange)
}

pub type CompileResult<T> = Result<T, CompileError>;

impl CompileError {
    pub fn mismatched_token(expected: TokenType, found: Token) -> CompileError {
        CompileError::MismatchedToken {
            expected,
            found
        }
    }

    pub fn unexpected_token(found: Token) -> CompileError {
        CompileError::UnexpectedToken(found)
    }

    pub fn unknown_form(form: String, range: SourceRange) -> CompileError {
        CompileError::UnknownForm(form, range)
    }

    pub fn unknown_note(note: String, range: SourceRange) -> CompileError {
        CompileError::UnknownNote(note, range)
    }
}
