#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum AudioField {
    Value
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum GraphField {
    Value,
    Speed
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum MidiField {
    Value
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum RollField {
    Value,
    Speed
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum ScopeField {
    Value
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum NumExtractField {
    Value
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum MidiExtractField {
    Value
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum ControlField {
    Audio(AudioField),
    Graph(GraphField),
    Midi(MidiField),
    Roll(RollField),
    Scope(ScopeField),
    NumExtract(NumExtractField),
    MidiExtract(MidiExtractField)
}
