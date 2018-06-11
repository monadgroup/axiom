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
