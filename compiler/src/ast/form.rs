use ast::SourceRange;

#[derive(Debug, PartialEq, Eq, Clone, Copy, Hash)]
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
