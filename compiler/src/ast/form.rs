use ast::{SourcePos};

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
    start_pos: SourcePos,
    end_pos: SourcePos,
    form_type: FormType
}

impl Form {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, form_type: FormType) -> Form {
        Form {
            start_pos,
            end_pos,
            form_type
        }
    }

    pub fn get_start_pos(&self) -> SourcePos {
        self.start_pos
    }

    pub fn get_end_pos(&self) -> SourcePos {
        self.end_pos
    }

    pub fn get_form_type(&self) -> FormType {
        self.form_type
    }
}
