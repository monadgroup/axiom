use codegen::util;
use inkwell::module::Module;
use inkwell::values::PointerValue;

pub fn get_sample_rate(module: &Module) -> PointerValue {
    util::get_or_create_global(
        module,
        "maxim.samplerate",
        &module.get_context().f32_type().vec_type(2),
    ).as_pointer_value()
}

pub fn get_bpm(module: &Module) -> PointerValue {
    util::get_or_create_global(
        module,
        "maxim.bpm",
        &module.get_context().f32_type().vec_type(2),
    ).as_pointer_value()
}
