#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum ControlType {
    Audio,
    Graph,
    Midi,
    Roll,
    Scope,
    NumExtract,
    MidiExtract
}
