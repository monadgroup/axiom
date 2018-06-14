use ast::{SourceRange, ControlType, UNDEF_SOURCE_RANGE};
use parser::{Token, TokenType};
use std::fmt::Write;

pub enum CompileError {
    MismatchedToken { expected: TokenType, found: Token },
    UnexpectedToken(Token),
    UnexpectedEnd,
    UnknownForm(String, SourceRange),
    UnknownNote(String, SourceRange),
    UnknownControl(String, SourceRange),
    UnknownField(ControlType, String, SourceRange),
    RequiredAssignable(SourceRange),
}

pub type CompileResult<T> = Result<T, CompileError>;

impl CompileError {
    pub fn mismatched_token(expected: TokenType, found: Token) -> CompileError {
        CompileError::MismatchedToken { expected, found }
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

    pub fn unknown_control(control: String, range: SourceRange) -> CompileError {
        CompileError::UnknownControl(control, range)
    }

    pub fn unknown_field(control: ControlType, field: String, range: SourceRange) -> CompileError {
        CompileError::UnknownField(control, field, range)
    }

    pub fn required_assignable(range: SourceRange) -> CompileError {
        CompileError::RequiredAssignable(range)
    }

    pub fn formatted(&self) -> (String, SourceRange) {
        let mut res = "".to_owned();
        let (result, range) = match self {
            CompileError::MismatchedToken { expected, found } => (write!(&mut res, "Dude, why is there a {:?}? I expected a {:?} here.", found.token_type, expected), found.pos),
            CompileError::UnexpectedToken(token) => (write!(&mut res, "Hey man, not cool. I didn't expect this {:?}!", token.token_type), token.pos),
            CompileError::UnexpectedEnd => (write!(&mut res, "Woah, hold your horses! I think you're missing something at the end there."), UNDEF_SOURCE_RANGE),
            CompileError::UnknownForm(form, range) => (write!(&mut res, "Come on man, I don't support {} forms.", form), *range),
            CompileError::UnknownNote(note, range) => (write!(&mut res, "Ey my man, don't you know that {} isn't a valid note?", note), *range),
            CompileError::UnknownControl(control, range) => (write!(&mut res, "Come on man, I don't support {} controls.", control), *range),
            CompileError::UnknownField(control, field, range) => (write!(&mut res, "Dude! {:?} controls don't have a {} field!", control, field), *range),
            CompileError::RequiredAssignable(range) => (write!(&mut res, "Hey! I need something I can assign to here, not this silly fudge you're giving me."), *range)
        };
        result.unwrap();
        (res, range)
    }
}
