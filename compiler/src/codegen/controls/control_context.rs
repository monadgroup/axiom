use codegen::BuilderContext;
use inkwell::values::PointerValue;

pub struct ControlContext<'a> {
    pub ctx: BuilderContext<'a>,
    pub val_ptr: PointerValue,
    pub data_ptr: PointerValue,
}

pub struct ControlUiContext<'a> {
    pub ctx: BuilderContext<'a>,
    pub val_ptr: PointerValue,
    pub data_ptr: PointerValue,
    pub ui_ptr: PointerValue,
}
