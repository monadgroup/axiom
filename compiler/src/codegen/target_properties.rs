use inkwell::targets::TargetMachine;

#[derive(Debug)]
pub struct TargetProperties {
    pub include_ui: bool,
    pub min_size: bool,
    pub machine: TargetMachine,
}

impl TargetProperties {
    pub fn new(include_ui: bool, min_size: bool, machine: TargetMachine) -> Self {
        TargetProperties {
            include_ui,
            min_size,
            machine,
        }
    }
}
