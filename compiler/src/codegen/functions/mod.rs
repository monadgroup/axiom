use codegen::LifecycleFunc;
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::types::StructType;
use inkwell::values::PointerValue;
use mir::block::Function;

pub enum FunctionLifecycleFunc {
    Construct,
    Destruct,
}

pub fn get_return_type(context: &Context, function: &Function) -> StructType {
    unimplemented!()
}

pub fn build_lifecycle_call(
    module: &Module,
    builder: &mut Builder,
    function: &Function,
    lifecycle: FunctionLifecycleFunc,
    data_ptr: PointerValue,
) {
    unimplemented!()
}

pub fn build_call(
    module: &Module,
    builder: &mut Builder,
    function: &Function,
    data_ptr: PointerValue,
    args: &[PointerValue],
    varargs: &[PointerValue],
    out_val: PointerValue,
) {
    unimplemented!()
}

/*pub trait Function {
    fn function_type() -> Function;

    fn data_type(context: &Context) -> StructType {
        context.struct_type(&[], false)
    }
}*/
