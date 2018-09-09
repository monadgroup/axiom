use super::ControlType;
use std::fmt;

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum AudioField {
    Value,
}

impl fmt::Display for AudioField {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            AudioField::Value => write!(f, "value"),
        }
    }
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum GraphField {
    Value,
    State,
}

impl fmt::Display for GraphField {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            GraphField::Value => write!(f, "value"),
            GraphField::State => write!(f, "state"),
        }
    }
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum MidiField {
    Value,
}

impl fmt::Display for MidiField {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            MidiField::Value => write!(f, "value"),
        }
    }
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum RollField {
    Value,
    Speed,
}

impl fmt::Display for RollField {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            RollField::Value => write!(f, "value"),
            RollField::Speed => write!(f, "speed"),
        }
    }
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum ScopeField {
    Value,
}

impl fmt::Display for ScopeField {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            ScopeField::Value => write!(f, "value"),
        }
    }
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum AudioExtractField {
    Value,
}

impl fmt::Display for AudioExtractField {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            AudioExtractField::Value => write!(f, "value"),
        }
    }
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum MidiExtractField {
    Value,
}

impl fmt::Display for MidiExtractField {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            MidiExtractField::Value => write!(f, "value"),
        }
    }
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum ControlField {
    Audio(AudioField),
    Graph(GraphField),
    Midi(MidiField),
    Roll(RollField),
    Scope(ScopeField),
    AudioExtract(AudioExtractField),
    MidiExtract(MidiExtractField),
}

impl fmt::Display for ControlField {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        let control_type = ControlType::from(*self);
        match self {
            ControlField::Audio(field) => write!(f, "{}.{}", control_type, field),
            ControlField::Graph(field) => write!(f, "{}.{}", control_type, field),
            ControlField::Midi(field) => write!(f, "{}.{}", control_type, field),
            ControlField::Roll(field) => write!(f, "{}.{}", control_type, field),
            ControlField::Scope(field) => write!(f, "{}.{}", control_type, field),
            ControlField::AudioExtract(field) => write!(f, "{}.{}", control_type, field),
            ControlField::MidiExtract(field) => write!(f, "{}.{}", control_type, field),
        }
    }
}
