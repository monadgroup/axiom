use super::{controls, converters, functions, intrinsics, math, values, TargetProperties};
use inkwell::module::Module;

pub fn codegen_lib(module: &Module, target: &TargetProperties) {
    controls::build_funcs(module, target);
    converters::build_funcs(module, target);
    functions::build_funcs(module, target);
    intrinsics::build_intrinsics(module, target);
    math::build_math_functions(module, target);
    values::MidiValue::initialize(module, target);
}
