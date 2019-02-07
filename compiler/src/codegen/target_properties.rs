use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::targets::TargetMachine;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum OptimizationLevel {
    Editor,

    None,
    Low,
    Medium,
    High,
    MinSize,
    AggressiveSize,
}

pub struct OptimizationSpecification {
    pub llvm_level: inkwell::OptimizationLevel,
    pub size_level: u32,
    pub inliner_threshold: u32,
}

impl OptimizationLevel {
    pub fn into_specification(self) -> OptimizationSpecification {
        match self {
            OptimizationLevel::Editor => OptimizationSpecification {
                llvm_level: inkwell::OptimizationLevel::Default,
                size_level: 0,
                inliner_threshold: 50,
            },
            OptimizationLevel::None => OptimizationSpecification {
                llvm_level: inkwell::OptimizationLevel::None,
                size_level: 0,
                inliner_threshold: 225,
            },
            OptimizationLevel::Low => OptimizationSpecification {
                llvm_level: inkwell::OptimizationLevel::Less,
                size_level: 0,
                inliner_threshold: 225,
            },
            OptimizationLevel::Medium => OptimizationSpecification {
                llvm_level: inkwell::OptimizationLevel::Default,
                size_level: 0,
                inliner_threshold: 225,
            },
            OptimizationLevel::High => OptimizationSpecification {
                llvm_level: inkwell::OptimizationLevel::Aggressive,
                size_level: 0,
                inliner_threshold: 250,
            },
            OptimizationLevel::MinSize => OptimizationSpecification {
                llvm_level: inkwell::OptimizationLevel::Default,
                size_level: 1,
                inliner_threshold: 50,
            },
            OptimizationLevel::AggressiveSize => OptimizationSpecification {
                llvm_level: inkwell::OptimizationLevel::Aggressive,
                size_level: 2,
                inliner_threshold: 5,
            },
        }
    }
}

#[derive(Debug)]
pub struct TargetProperties {
    pub include_ui: bool,
    pub optimization_level: OptimizationLevel,
    pub machine: TargetMachine,
}

impl TargetProperties {
    pub fn new(
        include_ui: bool,
        optimization_level: OptimizationLevel,
        machine: TargetMachine,
    ) -> Self {
        TargetProperties {
            include_ui,
            optimization_level,
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
