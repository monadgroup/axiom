use codegen::util;
use inkwell::module::Module;
use inkwell::types::ArrayType;
use inkwell::values::GlobalValue;
use mir::block::FUNCTION_TABLE;

pub const SAMPLERATE_GLOBAL_NAME: &str = "maxim.samplerate";
pub const BPM_GLOBAL_NAME: &str = "maxim.bpm";
pub const PROFILE_TIME_GLOBAL_NAME: &str = "maxim.profiletimes";

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

fn get_profile_time_type(module: &Module) -> ArrayType {
    module
        .get_context()
        .i64_type()
        .array_type(FUNCTION_TABLE.len() as u32)
}

pub fn get_profile_time(module: &Module) -> GlobalValue {
    util::get_or_create_global(
        module,
        PROFILE_TIME_GLOBAL_NAME,
        &get_profile_time_type(module),
    )
}

pub fn build_globals(module: &Module) {
    get_sample_rate(module).set_initializer(&util::get_vec_spread(&module.get_context(), 44100.));
    get_bpm(module).set_initializer(&util::get_vec_spread(&module.get_context(), 60.));
    get_profile_time(module).set_initializer(&get_profile_time_type(module).const_null());
}
