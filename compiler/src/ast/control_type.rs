use ast::ControlField;

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum ControlType {
    Audio,
    Graph,
    Midi,
    Roll,
    Scope,
    NumExtract,
    MidiExtract,
}

impl From<ControlField> for ControlType {
    fn from(field: ControlField) -> Self {
        match field {
            ControlField::Audio(_) => ControlType::Audio,
            ControlField::Graph(_) => ControlType::Graph,
            ControlField::Midi(_) => ControlType::Midi,
            ControlField::Roll(_) => ControlType::Roll,
            ControlField::Scope(_) => ControlType::Scope,
            ControlField::NumExtract(_) => ControlType::NumExtract,
            ControlField::MidiExtract(_) => ControlType::MidiExtract
        }
    }
}
