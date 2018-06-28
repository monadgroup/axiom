use codegen::{util, LifecycleFunc, TargetProperties};
use inkwell::module::Module;
use inkwell::values::FunctionValue;
use mir::Surface;

/*fn get_lifecycle_func(module: &Module, surface: &Surface, target: &TargetProperties, lifecycle: LifecycleFunc) -> FunctionValue {
    let func_name = format!("maxim.surface.{}.{}.{}", surface.id.id, surface.id.debug_name, lifecycle);
    util::get_or_create_func(module, &func_name, &|| {

    });
}

pub fn build_lifecycle_func(module: &Module, surface: &Surface, target: &TargetProperties, lifecycle: LifecycleFunc) {

}*/
