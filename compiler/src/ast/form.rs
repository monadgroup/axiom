use ast::SourceRange;

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
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
    Q
}

#[derive(Debug, Clone)]
pub struct Form {
    pos: SourceRange,
    form_type: FormType
}

impl Form {
    pub fn new(pos: SourceRange, form_type: FormType) -> Form {
        Form {
            pos,
            form_type
        }
    }

    pub fn pos(&self) -> &SourceRange { &self.pos }
    pub fn form_type(&self) -> FormType { self.form_type }
}
