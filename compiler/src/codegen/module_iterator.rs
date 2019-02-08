use inkwell::module::Module;
use inkwell::values::{FunctionValue, GlobalValue};

pub struct ModuleFunctionIterator {
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

pub struct ModuleGlobalIterator {
    next_global: Option<GlobalValue>,
}

impl ModuleGlobalIterator {
    pub fn new(module: &Module) -> ModuleGlobalIterator {
        ModuleGlobalIterator {
            next_global: module.get_first_global(),
        }
    }
}

impl Iterator for ModuleGlobalIterator {
    type Item = GlobalValue;

    fn next(&mut self) -> Option<GlobalValue> {
        match self.next_global {
            Some(global) => {
                self.next_global = global.get_next_global();
                Some(global)
            }
            None => None,
        }
    }
}
