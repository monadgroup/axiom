pub mod export_config;

use super::mir_optimizer;
use super::Transaction;
use crate::codegen::{
    block, data_analyzer, root, runtime_lib, surface, ModuleFunctionIterator, ModuleGlobalIterator,
    ObjectCache, Optimizer, TargetProperties,
};
use crate::{mir, pass};
use inkwell::context::Context;
use inkwell::module::Linkage;
use std::collections::{HashMap, HashSet};
use std::iter::FromIterator;
use std::path::Path;

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

fn print_surfaces(surfaces: &HashMap<mir::SurfaceRef, mir::Surface>) {
    for surface in surfaces.values() {
        println!("{}", surface);
    }
}

fn print_blocks(blocks: &HashMap<mir::BlockRef, mir::Block>) {
    for block in blocks.values() {
        println!("{}", block);
    }
}

pub fn build_module_from_transaction(target: &TargetProperties, transaction: Transaction) {
    let optimizer = Optimizer::new(target);
    let context = Context::create();
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

    println!(
        ">> Before block dedup, there are {} blocks",
        prepared_blocks.len()
    );
    pass::deduplicate_blocks(&mut prepared_blocks, prepared_surfaces.values_mut());
    println!(
        ">> After block dedup, there are {} blocks",
        prepared_blocks.len()
    );
    println!(
        ">> Before surface dedup, there are {} surfaces",
        prepared_surfaces.len()
    );
    pass::deduplicate_surfaces(&mut prepared_surfaces);
    println!(
        ">> After surface dedup, there are {} surfaces",
        prepared_surfaces.len()
    );
    pass::flatten_groups(&mut prepared_surfaces);
    println!(
        ">> After flatten groups, there are {} surfaces",
        prepared_surfaces.len()
    );

    print_blocks(&prepared_blocks);
    print_surfaces(&prepared_surfaces);

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
    runtime_lib::codegen_lib(&export_module, target);

    for block in prepared_blocks.values() {
        block::build_funcs(&export_module, &cache, block);
    }
    for surface in prepared_surfaces.values() {
        surface::build_funcs(&export_module, &cache, surface);
    }

    // build the root
    if let Some(root) = transaction.root {
        let initialized_global =
            root::build_initialized_global(&export_module, &cache, 0, "maxim.export.initialized");
        let scratch_global =
            root::build_scratch_global(&export_module, &cache, 0, "maxim.export.scratch");
        let sockets_global = root::build_sockets_global(
            &export_module,
            &root,
            "maxim.export.sockets",
            "maxim.export.portals",
        );
        let pointers_global = root::build_pointers_global(
            &export_module,
            &cache,
            0,
            "maxim.export.pointers",
            initialized_global.as_pointer_value(),
            scratch_global.as_pointer_value(),
            sockets_global.sockets.as_pointer_value(),
        );
        root::build_funcs(
            &export_module,
            &cache,
            0,
            "maxim_construct",
            "maxim_update",
            "maxim_destruct",
            pointers_global.as_pointer_value(),
        );
    }

    // make all functions and globals that start with maxim.* be private
    for func in ModuleFunctionIterator::new(&export_module) {
        if func.get_name().to_str().unwrap().starts_with("maxim.") {
            func.set_linkage(Linkage::PrivateLinkage);
        }
    }
    for global in ModuleGlobalIterator::new(&export_module) {
        if global.get_name().to_str().unwrap().starts_with("maxim.") {
            global.set_linkage(Linkage::PrivateLinkage);
        }
    }

    optimizer.optimize_module(&export_module);
    export_module
        .print_to_file(&Path::new("test_export.ll"))
        .unwrap();
}

/// Runs necessary preparation passes on the surfaces
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
