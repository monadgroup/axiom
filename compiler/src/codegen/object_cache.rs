use codegen::{data_analyzer, TargetProperties};
use inkwell::context::Context;
use mir::{Block, BlockRef, Surface, SurfaceRef};

pub trait ObjectCache {
    fn context(&self) -> &Context;

    fn target(&self) -> &TargetProperties;

    fn surface_mir(&self, id: SurfaceRef) -> Option<&Surface>;

    fn surface_layout(&self, id: SurfaceRef) -> Option<&data_analyzer::SurfaceLayout>;

    fn block_mir(&self, id: BlockRef) -> Option<&Block>;

    fn block_layout(&self, id: BlockRef) -> Option<&data_analyzer::BlockLayout>;
}
