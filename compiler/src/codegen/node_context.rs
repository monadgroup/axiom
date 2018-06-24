use super::data_analyzer::BlockMetadata;
use super::BuilderContext;
use inkwell::values::PointerValue;

pub struct NodeContext<'a> {
    pub ctx: BuilderContext<'a>,
    pub meta: BlockMetadata,
    pub ui_meta: Option<BlockMetadata>,
}

impl<'a> NodeContext<'a> {
    pub fn get_statement(&self, _index: usize) -> PointerValue {
        unimplemented!()
    }
}
