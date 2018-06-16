use ast::{ControlType, SourceRange, UNDEF_SOURCE_RANGE};
use mir::{VarType, block::FunctionArgRange};
use parser::{Token, TokenType};
use std::fmt::Write;

#[derive(Debug, Clone)]
pub enum CompileError {
    MismatchedToken {
        expected: TokenType,
        found: Token,
    },
    UnexpectedToken(Token),
    UnexpectedEnd,
    UnknownForm(String, SourceRange),
    UnknownNote(String, SourceRange),
    UnknownControl(String, SourceRange),
    UnknownField(ControlType, String, SourceRange),
    RequiredAssignable(SourceRange),
    UnmatchedTuples(usize, usize, SourceRange),
    MismatchedType {
        expected: VarType,
        found: VarType,
        range: SourceRange,
    },
    AccessOutOfBounds {
        actual_count: usize,
        index: usize,
        range: SourceRange,
    },
    UnknownVariable(String, SourceRange),
    UnknownFunction(String, SourceRange),
    MismatchedArgCount(FunctionArgRange, usize, SourceRange)
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

    pub fn unmatched_tuples(left_len: usize, right_len: usize, range: SourceRange) -> CompileError {
        CompileError::UnmatchedTuples(left_len, right_len, range)
    }

    pub fn mismatched_type(expected: VarType, found: VarType, range: SourceRange) -> CompileError {
        CompileError::MismatchedType {
            expected,
            found,
            range,
        }
    }

    pub fn access_out_of_bounds(
        actual_count: usize,
        index: usize,
        range: SourceRange,
    ) -> CompileError {
        CompileError::AccessOutOfBounds {
            actual_count,
            index,
            range,
        }
    }

    pub fn unknown_variable(name: String, range: SourceRange) -> CompileError {
        CompileError::UnknownVariable(name, range)
    }

    pub fn unknown_function(name: String, range: SourceRange) -> CompileError {
        CompileError::UnknownFunction(name, range)
    }

    pub fn mismatched_arg_count(expected: FunctionArgRange, provided: usize, range: SourceRange) -> CompileError {
        CompileError::MismatchedArgCount(expected, provided, range)
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
            CompileError::RequiredAssignable(range) => (write!(&mut res, "Hey! I need something I can assign to here, not this silly fudge you're giving me."), *range),
            CompileError::UnmatchedTuples(left_len, right_len, range) => (write!(&mut res, "OOOOOOOOOOOOOOOOOOOOOOYYYYYY!!!!1! You're trying to assign {} values to {} ones!", right_len, left_len), *range),
            CompileError::MismatchedType { expected, found, range } => (write!(&mut res, "Oyyyy m80, I need a {:?} here, not this bad boi {:?}!", expected, found), *range),
            CompileError::AccessOutOfBounds { actual_count, index, range } => (write!(&mut res, "Ohh hekkers, there's nothing at index {} in an {}-sized tuple!", index, actual_count), *range),
            CompileError::UnknownVariable(name, range) => (write!(&mut res, "Ah hekkers mah dude! {} hasn't been set yet!", name), *range),
            CompileError::UnknownFunction(name, range) => (write!(&mut res, "WHAT IS THIS??!?! {} is def not a valid function :(", name), *range),
            CompileError::MismatchedArgCount(expected, provided, range) if *provided == 1 => (write!(&mut res, "Eyy! My dude, you're calling that function with 1 argument, but it needs {:?}!", expected), *range),
            CompileError::MismatchedArgCount(expected, provided, range) => (write!(&mut res, "Eyy! My dude, you're calling that function with {} arguments, but it needs {:?}!", provided, expected), *range)
        };
        result.unwrap();
        (res, range)
    }
}
