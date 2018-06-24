use super::BuilderContext;
use inkwell::values::PointerValue;

pub struct NodeContext<'a> {
    pub ctx: BuilderContext<'a>,
}

impl<'a> NodeContext<'a> {
    pub fn get_statement(&self, _index: usize) -> PointerValue {
        unimplemented!()
    }
}
