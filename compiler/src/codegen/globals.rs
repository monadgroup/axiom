use codegen::util;
use inkwell::module::Module;
use inkwell::values::GlobalValue;

pub const SAMPLERATE_GLOBAL_NAME: &str = "maxim.samplerate";
pub const BPM_GLOBAL_NAME: &str = "maxim.bpm";

pub fn get_sample_rate(module: &Module) -> GlobalValue {
    util::get_or_create_global(
        module,
        SAMPLERATE_GLOBAL_NAME,
        &module.get_context().f32_type().vec_type(2),
    )
}

pub fn get_bpm(module: &Module) -> GlobalValue {
    util::get_or_create_global(
        module,
        BPM_GLOBAL_NAME,
        &module.get_context().f32_type().vec_type(2),
    )
}

pub fn build_globals(module: &Module) {
    get_sample_rate(module).set_initializer(&util::get_vec_spread(&module.get_context(), 44100.));
    get_bpm(module).set_initializer(&util::get_vec_spread(&module.get_context(), 60.));
}
