use crate::codegen::util;
use crate::mir::block::FUNCTION_TABLE;
use inkwell::module::Module;
use inkwell::types::{ArrayType, VectorType};
use inkwell::values::GlobalValue;

pub const SAMPLERATE_GLOBAL_NAME: &str = "maxim.samplerate";
pub const BPM_GLOBAL_NAME: &str = "maxim.bpm";
pub const RAND_SEED_GLOBAL_NAME: &str = "maxim.randseed";
pub const PROFILE_TIME_GLOBAL_NAME: &str = "maxim.profiletimes";

pub fn get_sample_rate(module: &Module) -> GlobalValue {
    util::get_or_create_global(
        module,
        SAMPLERATE_GLOBAL_NAME,
        &module.get_context().f64_type().vec_type(2),
    )
}

pub fn get_bpm(module: &Module) -> GlobalValue {
    util::get_or_create_global(
        module,
        BPM_GLOBAL_NAME,
        &module.get_context().f64_type().vec_type(2),
    )
}

pub fn get_rand_seed(module: &Module) -> GlobalValue {
    util::get_or_create_global(
        module,
        RAND_SEED_GLOBAL_NAME,
        &module.get_context().i64_type().vec_type(2),
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
    let context = module.get_context();

    get_sample_rate(module).set_initializer(&util::get_vec_spread(&context, 44100.));
    get_bpm(module).set_initializer(&util::get_vec_spread(&context, 60.));
    get_rand_seed(module).set_initializer(&VectorType::const_vector(&[
        &context.i64_type().const_int(1, false),
        &context.i64_type().const_int(31337, false),
    ]));
    get_profile_time(module).set_initializer(&get_profile_time_type(module).const_null());
}
