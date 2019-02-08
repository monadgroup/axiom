use crate::ast::SourceRange;
use std::fmt;

#[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Clone, Copy, Hash)]
#[repr(u8)]
pub enum FormType {
    None,
    Control,
    Oscillator,
    Note,
    Frequency,
    Beats,
    Seconds,
    Samples,
    Db,
    Amplitude,
    Q,
}

#[derive(Debug, Clone)]
pub struct Form {
    pub pos: SourceRange,
    pub form_type: FormType,
}

impl Form {
    pub fn new(pos: SourceRange, form_type: FormType) -> Form {
        Form { pos, form_type }
    }
}

impl fmt::Display for FormType {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            FormType::None => write!(f, "none"),
            FormType::Control => write!(f, "control"),
            FormType::Oscillator => write!(f, "oscillator"),
            FormType::Note => write!(f, "note"),
            FormType::Frequency => write!(f, "frequency"),
            FormType::Beats => write!(f, "beats"),
            FormType::Seconds => write!(f, "seconds"),
            FormType::Samples => write!(f, "samples"),
            FormType::Db => write!(f, "db"),
            FormType::Amplitude => write!(f, "amplitude"),
            FormType::Q => write!(f, "q"),
        }
    }
}
