use codegen::BuilderContext;
use inkwell::values::PointerValue;

pub struct FunctionContext<'a> {
    pub ctx: BuilderContext<'a>,
    pub data_ptr: PointerValue,
}
