use inkwell::context::Context;
use inkwell::module::Module;
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

    pub fn create_module(&self, context: &Context, name: &str) -> Module {
        let module = context.create_module(name);
        module.set_target(&self.machine.get_triple().to_string_lossy());
        module.set_data_layout(&self.machine.get_data().get_data_layout());
        module
    }
}
