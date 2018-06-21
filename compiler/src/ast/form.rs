use ast::SourceRange;

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
