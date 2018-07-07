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
        println!(
            "{:?} {:?} {:?} {:?} {:?}",
            machine.get_triple(),
            machine.get_cpu(),
            machine.get_feature_string(),
            machine.get_target().get_name(),
            machine.get_target().get_description()
        );

        Jit {
            orc: Orc::new(machine),
        }
    }

    pub fn deploy(&self, module: &Module) -> JitKey {
        self.orc.add_module(module)
    }

    pub fn remove(&self, key: JitKey) {
        self.orc.remove_module(key);
    }

    pub fn get_symbol_address(&self, symbol: &str) -> u64 {
        self.orc.get_symbol_address(symbol)
    }
}
