use ast::{ControlType, SourceRange, UNDEF_SOURCE_RANGE};
use mir::{block::FunctionArgRange, VarType};
use parser::{Token, TokenType};
use std::fmt;

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
    MismatchedArgCount(FunctionArgRange, usize, SourceRange),
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

    pub fn mismatched_arg_count(
        expected: FunctionArgRange,
        provided: usize,
        range: SourceRange,
    ) -> CompileError {
        CompileError::MismatchedArgCount(expected, provided, range)
    }

    pub fn range(&self) -> SourceRange {
        match self {
            CompileError::MismatchedToken { found, .. } => found.pos,
            CompileError::UnexpectedToken(token) => token.pos,
            CompileError::UnexpectedEnd => UNDEF_SOURCE_RANGE,
            CompileError::UnknownForm(_, range) => *range,
            CompileError::UnknownNote(_, range) => *range,
            CompileError::UnknownControl(_, range) => *range,
            CompileError::UnknownField(_, _, range) => *range,
            CompileError::RequiredAssignable(range) => *range,
            CompileError::UnmatchedTuples(_, _, range) => *range,
            CompileError::MismatchedType { range, .. } => *range,
            CompileError::AccessOutOfBounds { range, .. } => *range,
            CompileError::UnknownVariable(_, range) => *range,
            CompileError::UnknownFunction(_, range) => *range,
            CompileError::MismatchedArgCount(_, _, range) => *range,
        }
    }
}

impl fmt::Display for CompileError {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            CompileError::MismatchedToken { expected, found } => write!(f, "Dude, why is there a {:?}? I expected a {:?} here.", found.token_type, expected),
            CompileError::UnexpectedToken(token) => write!(f, "Hey man, not cool. I didn't expect this {:?}!", token.token_type),
            CompileError::UnexpectedEnd => write!(f, "Woah, hold your horses! I think you're missing something at the end there."),
            CompileError::UnknownForm(form, _) => write!(f, "Come on man, I don't support {} forms.", form),
            CompileError::UnknownNote(note, _) => write!(f, "Ey my man, don't you know that {} isn't a valid note?", note),
            CompileError::UnknownControl(control, _) => write!(f, "Come on man, I don't support {} controls.", control),
            CompileError::UnknownField(control, field, _) => write!(f, "Dude! {:?} controls don't have a {} field!", control, field),
            CompileError::RequiredAssignable(_) => write!(f, "Hey! I need something I can assign to here, not this silly fudge you're giving me."),
            CompileError::UnmatchedTuples(left_len, right_len, _) => write!(f, "OOOOOOOOOOOOOOOOOOOOOOYYYYYY!!!!1! You're trying to assign {} values to {} ones!", right_len, left_len),
            CompileError::MismatchedType { expected, found, .. } => write!(f, "Oyyyy m80, I need a {:?} here, not this bad boi {:?}!", expected, found),
            CompileError::AccessOutOfBounds { actual_count, index, .. } => write!(f, "Ohh hekkers, there's nothing at index {} in an {}-element tuple!", index, actual_count),
            CompileError::UnknownVariable(name, _) => write!(f, "Ah hekkers mah dude! {} hasn't been set yet!", name),
            CompileError::UnknownFunction(name, _) => write!(f, "WHAT IS THIS??!?! {} is def not a valid function :(", name),
            CompileError::MismatchedArgCount(expected, provided, _) if *provided == 1 => write!(f, "Eyy! My dude, you're calling that function with 1 argument, but it needs {}!", expected),
            CompileError::MismatchedArgCount(expected, provided, _) => write!(f, "Eyy! My dude, you're calling that function with {} arguments, but it needs {}!", provided, expected)
        }
    }
}
