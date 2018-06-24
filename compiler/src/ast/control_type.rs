use ast::ControlField;
use std::fmt;

#[derive(Debug, PartialEq, Eq, Hash, Clone, Copy)]
pub enum ControlType {
    Audio,
    Graph,
    Midi,
    Roll,
    Scope,
    AudioExtract,
    MidiExtract,
}

impl ControlType {
    pub fn is_extract(&self) -> bool {
        *self == ControlType::AudioExtract || *self == ControlType::MidiExtract
    }
}

impl From<ControlField> for ControlType {
    fn from(field: ControlField) -> Self {
        match field {
            ControlField::Audio(_) => ControlType::Audio,
            ControlField::Graph(_) => ControlType::Graph,
            ControlField::Midi(_) => ControlType::Midi,
            ControlField::Roll(_) => ControlType::Roll,
            ControlField::Scope(_) => ControlType::Scope,
            ControlField::AudioExtract(_) => ControlType::AudioExtract,
            ControlField::MidiExtract(_) => ControlType::MidiExtract,
        }
    }
}

impl fmt::Display for ControlType {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            ControlType::Audio => write!(f, "audio"),
            ControlType::Graph => write!(f, "graph"),
            ControlType::Midi => write!(f, "midi"),
            ControlType::Roll => write!(f, "roll"),
            ControlType::Scope => write!(f, "scope"),
            ControlType::AudioExtract => write!(f, "audioextract"),
            ControlType::MidiExtract => write!(f, "midiextract"),
        }
    }
}
