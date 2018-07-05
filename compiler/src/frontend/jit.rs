use inkwell::module::Module;
use inkwell::orc::{Orc, OrcError, OrcModuleKey};
use inkwell::targets::{CodeModel, RelocMode, Target, TargetMachine};
use inkwell::OptimizationLevel;

pub type JitKey = OrcModuleKey;

#[derive(Debug)]
pub struct Jit {
    orc: Orc,
}

impl Jit {
    pub fn new() -> Self {
        let triple_llvm = TargetMachine::get_default_triple();
        let triple = triple_llvm.to_string_lossy();
        let target = Target::from_triple(&triple).unwrap();
        let machine = target
            .create_target_machine(
                &triple,
                "",
                "",
                OptimizationLevel::Aggressive,
                RelocMode::Default,
                CodeModel::Default,
            )
            .unwrap();

        Jit {
            orc: Orc::new(machine),
        }
    }

    pub fn deploy(&self, module: &Module) -> JitKey {
        self.orc.add_module(module, false).unwrap()
    }

    pub fn remove(&self, key: JitKey) {
        self.orc.remove_module(key).unwrap();
    }

    pub fn get_symbol_address(&self, symbol: &str) -> Result<u64, OrcError> {
        let mangled_name = self.orc.mangle_symbol(symbol);
        self.orc
            .get_symbol_address(mangled_name.to_string_lossy().as_ref())
    }
}
