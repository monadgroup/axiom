use super::build_meta_output::ModuleMetadata;
use crate::codegen::{block, data_analyzer, root, surface, ObjectCache, TargetProperties};
use crate::frontend::{mir_optimizer, Transaction};
use crate::{mir, pass};
use inkwell::context::Context;
use inkwell::module::Module;
use std::collections::{HashMap, HashSet};
use std::iter::FromIterator;

struct ExportObjectCache<'context, 'target, 'mir> {
    context: &'context Context,
    target: &'target TargetProperties,
    surface_mir: &'mir HashMap<mir::SurfaceRef, mir::Surface>,
    block_mir: &'mir HashMap<mir::BlockRef, mir::Block>,
    surface_layout: &'mir HashMap<mir::SurfaceRef, data_analyzer::SurfaceLayout>,
    block_layout: &'mir HashMap<mir::BlockRef, data_analyzer::BlockLayout>,
}

impl ObjectCache for ExportObjectCache<'_, '_, '_> {
    fn context(&self) -> &Context {
        self.context
    }

    fn target(&self) -> &TargetProperties {
        self.target
    }

    fn surface_mir(&self, id: mir::SurfaceRef) -> Option<&mir::Surface> {
        self.surface_mir.get(&id)
    }

    fn surface_layout(&self, id: mir::SurfaceRef) -> Option<&data_analyzer::SurfaceLayout> {
        self.surface_layout.get(&id)
    }

    fn block_mir(&self, id: mir::BlockRef) -> Option<&mir::Block> {
        self.block_mir.get(&id)
    }

    fn block_layout(&self, id: mir::BlockRef) -> Option<&data_analyzer::BlockLayout> {
        self.block_layout.get(&id)
    }
}

pub fn build_instrument_module(
    context: &Context,
    target: &TargetProperties,
    transaction: Transaction,
    module_meta: &ModuleMetadata,
) -> Module {
    let mut id_allocator = mir::IncrementalIdAllocator::new(0);

    // Reserve all of the currently-used IDs in the allocator, so we don't get duplicates when
    // generating new IDs in passes.
    for &used_id in transaction
        .surfaces
        .keys()
        .into_iter()
        .chain(transaction.blocks.keys().into_iter())
    {
        id_allocator.reserve(used_id);
    }

    let mut prepared_surfaces = prepare_surfaces(
        transaction.surfaces.into_iter().map(|(_, surface)| surface),
        &mut id_allocator,
    );
    let mut prepared_blocks = prepare_blocks(transaction.blocks);

    pass::sort_group_sockets(&mut prepared_surfaces);
    pass::deduplicate_blocks(&mut prepared_blocks, prepared_surfaces.values_mut());
    pass::deduplicate_surfaces(&mut prepared_surfaces);
    pass::flatten_groups(&mut prepared_surfaces);

    let block_layouts = build_block_layouts(&context, target, prepared_blocks.values());
    let mut surface_layouts = HashMap::new();
    build_surface_layouts(
        &context,
        target,
        &prepared_surfaces,
        &prepared_blocks,
        &mut surface_layouts,
        &block_layouts,
    );

    let cache = ExportObjectCache {
        context: &context,
        target,
        surface_mir: &prepared_surfaces,
        block_mir: &prepared_blocks,
        surface_layout: &surface_layouts,
        block_layout: &block_layouts,
    };

    let export_module = target.create_module(&context, "export");

    for block in prepared_blocks.values() {
        block::build_funcs(&export_module, &cache, block);
    }
    for surface in prepared_surfaces.values() {
        surface::build_funcs(&export_module, &cache, surface);
    }

    build_root(
        &export_module,
        module_meta,
        &cache,
        &transaction.root.unwrap(),
    );

    export_module
}

fn build_root(
    module: &Module,
    module_meta: &ModuleMetadata,
    cache: &dyn ObjectCache,
    root: &mir::Root,
) {
    let initialized_global =
        root::build_initialized_global(&module, cache, 0, "maxim.data.initialized");
    initialized_global.set_constant(true);
    let scratch_global = root::build_scratch_global(&module, cache, 0, "maxim.data.scratch");
    let sockets_global = root::build_sockets_global(
        &module,
        root,
        "maxim.data.portals",
        "maxim.data.portals.ptr",
    );
    let pointers_global = root::build_pointers_global(
        &module,
        cache,
        0,
        "maxim.data.pointers",
        initialized_global.as_pointer_value(),
        scratch_global.as_pointer_value(),
        sockets_global.sockets.as_pointer_value(),
    );
    root::build_funcs(
        &module,
        cache,
        0,
        &module_meta.init_func_name,
        &module_meta.generate_func_name,
        &module_meta.cleanup_func_name,
        pointers_global.as_pointer_value(),
    );
}

fn prepare_surfaces(
    surfaces: impl IntoIterator<Item = mir::Surface>,
    allocator: &mut mir::IdAllocator,
) -> HashMap<mir::SurfaceRef, mir::Surface> {
    HashMap::from_iter(
        mir_optimizer::prepare_surfaces(surfaces, allocator)
            .map(|mut surface| {
                pass::sort_value_groups(&mut surface);
                surface
            })
            .map(|surface| (surface.id.id, surface)),
    )
}

fn prepare_blocks(
    mut blocks: HashMap<mir::BlockRef, mir::Block>,
) -> HashMap<mir::BlockRef, mir::Block> {
    mir_optimizer::prepare_blocks(blocks.values_mut());
    blocks
}

fn build_block_layouts<'block>(
    context: &Context,
    target: &TargetProperties,
    blocks: impl IntoIterator<Item = &'block mir::Block>,
) -> HashMap<mir::BlockRef, data_analyzer::BlockLayout> {
    HashMap::from_iter(blocks.into_iter().map(|block| {
        (
            block.id.id,
            data_analyzer::build_block_layout(context, block, target),
        )
    }))
}

fn build_surface_layouts(
    context: &Context,
    target: &TargetProperties,
    surface_mirs: &HashMap<mir::SurfaceRef, mir::Surface>,
    block_mirs: &HashMap<mir::BlockRef, mir::Block>,
    surface_layouts: &mut HashMap<mir::SurfaceRef, data_analyzer::SurfaceLayout>,
    block_layouts: &HashMap<mir::BlockRef, data_analyzer::BlockLayout>,
) {
    let mut visited_surfaces = HashSet::new();
    visit_surface(
        0,
        &mut visited_surfaces,
        context,
        target,
        surface_mirs,
        block_mirs,
        surface_layouts,
        block_layouts,
    );
}

fn visit_surface(
    surface_id: mir::SurfaceRef,
    visited_surfaces: &mut HashSet<mir::SurfaceRef>,
    context: &Context,
    target: &TargetProperties,
    surface_mirs: &HashMap<mir::SurfaceRef, mir::Surface>,
    block_mirs: &HashMap<mir::BlockRef, mir::Block>,
    surface_layouts: &mut HashMap<mir::SurfaceRef, data_analyzer::SurfaceLayout>,
    block_layouts: &HashMap<mir::BlockRef, data_analyzer::BlockLayout>,
) {
    if !visited_surfaces.insert(surface_id) {
        return;
    }

    let surface = &surface_mirs[&surface_id];
    for node in &surface.nodes {
        let subsurface_id = match node.data {
            mir::NodeData::Group(surface_id) => surface_id,
            mir::NodeData::ExtractGroup { surface, .. } => surface,
            _ => continue,
        };

        visit_surface(
            subsurface_id,
            visited_surfaces,
            context,
            target,
            surface_mirs,
            block_mirs,
            surface_layouts,
            block_layouts,
        );
    }

    // all dependencies have their layouts built now, so we can build ours
    let new_layout = data_analyzer::build_surface_layout(
        &ExportObjectCache {
            context,
            target,
            surface_mir: surface_mirs,
            block_mir: block_mirs,
            surface_layout: surface_layouts,
            block_layout: block_layouts,
        },
        surface,
    );
    surface_layouts.insert(surface_id, new_layout);
}
