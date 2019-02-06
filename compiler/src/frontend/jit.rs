use inkwell::module::Module;
use inkwell::orc::{Orc, OrcModuleKey};
use inkwell::targets::TargetMachine;

pub type JitKey = OrcModuleKey;

#[derive(Debug)]
pub struct Jit {
    orc: Orc,
}

impl Jit {
    pub fn new() -> Self {
        let machine = TargetMachine::select();
        let orc = Orc::new(machine);

        Jit { orc }
    }

    pub fn deploy(&self, module: &Module) -> JitKey {
        self.orc.add_module(&module.clone())
    }

    pub fn remove(&self, key: JitKey) {
        self.orc.remove_module(key);
    }

    pub fn get_symbol_address(&self, symbol: &str) -> u64 {
        self.orc.get_symbol_address(symbol)
    }
}

impl Default for Jit {
    fn default() -> Self {
        Self::new()
    }
}
