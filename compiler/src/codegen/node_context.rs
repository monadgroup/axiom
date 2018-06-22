use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::PointerValue;

pub struct NodeContext<'a> {
    pub context: &'a Context,
    pub module: &'a Module,
    pub alloca_builder: &'a mut Builder,
    pub builder: &'a mut Builder,
}

impl<'a> NodeContext<'a> {
    pub fn get_statement(&self, index: usize) -> PointerValue {
        unimplemented!()
    }
}
