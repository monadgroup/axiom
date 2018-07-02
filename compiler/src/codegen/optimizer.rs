use codegen::TargetProperties;
use inkwell::module::Module;
use inkwell::passes::{PassManager, PassManagerBuilder};
use inkwell::values::FunctionValue;
use inkwell::OptimizationLevel;

struct ModuleFunctionIterator {
    next_func: Option<FunctionValue>,
}

impl ModuleFunctionIterator {
    pub fn new(module: &Module) -> ModuleFunctionIterator {
        ModuleFunctionIterator {
            next_func: module.get_first_function(),
        }
    }
}

impl Iterator for ModuleFunctionIterator {
    type Item = FunctionValue;

    fn next(&mut self) -> Option<FunctionValue> {
        match self.next_func {
            Some(func) => {
                self.next_func = func.get_next_function();
                Some(func)
            }
            None => None,
        }
    }
}

#[derive(Debug)]
pub struct Optimizer {
    builder: PassManagerBuilder,
}

impl Optimizer {
    pub fn new(target: &TargetProperties) -> Self {
        let builder = PassManagerBuilder::create();

        if target.min_size {
            builder.set_optimization_level(OptimizationLevel::Default);
            builder.set_size_level(2);

            // threshold for -Oz, see http://llvm.org/doxygen/InlineCost_8h_source.html
            builder.set_inliner_with_threshold(5);
        } else {
            builder.set_optimization_level(OptimizationLevel::Aggressive);
            builder.set_size_level(0);

            // threshold for -O3, see http://llvm.org/doxygen/InlineCost_8h_source.html
            builder.set_inliner_with_threshold(250);
        }

        Optimizer { builder }
    }

    pub fn optimize_module(&self, module: &Module) {
        let module_pass = PassManager::create_for_module();
        self.builder.populate_module_pass_manager(&module_pass);

        let func_pass = PassManager::create_for_function(module);
        self.builder.populate_function_pass_manager(&func_pass);

        func_pass.initialize();
        let func_iterator = ModuleFunctionIterator::new(module);
        for func in func_iterator {
            func_pass.run_on_function(&func);
        }
        module_pass.run_on_module(module);
    }
}
