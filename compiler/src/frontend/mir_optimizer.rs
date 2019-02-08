use crate::codegen::TargetProperties;
use crate::{mir, pass};
use std::iter;

/// Run basic passes on surfaces that are necessary for regular operation:
///   - Find extract regions and move them into extract groups
///   - Remove dead value groups (which can appear from the extractor grouping pass)
///   - Adjust order of nodes in the surface
pub fn prepare_surfaces<'iter>(
    surfaces: impl IntoIterator<Item = mir::Surface> + 'iter,
    id_allocator: &'iter mut mir::IdAllocator,
    target: &'iter TargetProperties,
) -> impl Iterator<Item = mir::Surface> + 'iter {
    surfaces
        .into_iter()
        .flat_map(move |mut surface| {
            let new_surfaces = pass::group_extracted(&mut surface, id_allocator);
            new_surfaces.into_iter().chain(iter::once(surface))
        })
        .map(move |mut surface| {
            pass::order_nodes(&mut surface, target);
            pass::remove_dead_groups(&mut surface);
            surface
        })
}

pub fn prepare_blocks<'block>(blocks: impl IntoIterator<Item = &'block mut mir::Block>) {
    for block in blocks {
        pass::remove_dead_code(block);
    }
}
