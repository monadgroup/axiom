use ast::SourceRange;
use std::fmt;

#[derive(Debug, PartialEq, Eq, Clone, Copy, Hash)]
pub enum FormType {
    None = 0,
    Control = 1,
    Oscillator = 2,
    Note = 3,
    Frequency = 4,
    Beats = 5,
    Seconds = 6,
    Samples = 7,
    Db = 8,
    Amplitude = 9,
    Q = 10,
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
