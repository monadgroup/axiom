use super::data_analyzer::BlockLayout;
use super::BuilderContext;
use inkwell::values::PointerValue;

pub struct NodeContext<'a> {
    pub ctx: BuilderContext<'a>,
    pub layout: BlockLayout,
}

impl<'a> NodeContext<'a> {
    pub fn get_statement(&self, _index: usize) -> PointerValue {
        unimplemented!()
    }

    pub fn get_data_entry(&self, _index: usize) -> PointerValue {
        unimplemented!()
    }

    pub fn get_group_entry(&self, _index: usize) -> PointerValue {
        unimplemented!()
    }

    pub fn get_ui_entry(&self, _index: usize) -> Option<PointerValue> {
        unimplemented!()
    }
}
