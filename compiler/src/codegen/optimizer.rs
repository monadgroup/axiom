use crate::codegen::{ModuleFunctionIterator, TargetProperties};
use inkwell::module::Module;
use inkwell::passes::{PassManager, PassManagerBuilder};

#[derive(Debug)]
pub struct Optimizer {
    module_pass: PassManager,
    builder: PassManagerBuilder,
}

impl Optimizer {
    pub fn new(target: &TargetProperties) -> Self {
        let builder = PassManagerBuilder::create();
        let opt_specification = target.optimization_level.into_specification();
        builder.set_optimization_level(opt_specification.llvm_level);
        builder.set_size_level(opt_specification.size_level);
        builder.set_inliner_with_threshold(opt_specification.inliner_threshold);

        let module_pass = PassManager::create_for_module();
        builder.populate_module_pass_manager(&module_pass);
        target.machine.add_analysis_passes(&module_pass);

        Optimizer {
            module_pass,
            builder,
        }
    }

    pub fn optimize_module(&self, module: &Module) {
        if let Err(err) = module.verify() {
            module.print_to_stderr();
            panic!(err.to_string());
        }

        let func_pass = PassManager::create_for_function(module);
        self.builder.populate_function_pass_manager(&func_pass);

        func_pass.initialize();
        let func_iterator = ModuleFunctionIterator::new(module);
        for func in func_iterator {
            func_pass.run_on_function(&func);
        }
        self.module_pass.run_on_module(module);
    }
}
