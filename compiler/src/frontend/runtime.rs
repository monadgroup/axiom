use super::Transaction;
use codegen::{block, data_analyzer, surface, ObjectCache, Optimizer, TargetProperties};
use inkwell::context::Context;
use inkwell::module::Module;
use mir::{Block, BlockRef, IdAllocator, Surface, SurfaceRef};
use pass;
use std::collections::HashMap;
use time;

#[derive(Debug)]
pub struct Runtime {
    next_id: u64,
    context: Context,
    target: TargetProperties,
    pub optimizer: Optimizer,
    surfaces: HashMap<SurfaceRef, (Surface, data_analyzer::SurfaceLayout, Module)>,
    blocks: HashMap<BlockRef, (Block, data_analyzer::BlockLayout, Module)>,
}

impl Runtime {
    pub fn new(target: TargetProperties) -> Self {
        let optimizer = Optimizer::new(&target);
        Runtime {
            next_id: 1,
            context: Context::create(),
            target,
            optimizer,
            surfaces: HashMap::new(),
            blocks: HashMap::new(),
        }
    }

    /// Applies a set of surfaces and blocks to the runtime. Note that surfaces _must_ be ordered
    /// in reverse-dependency order (that is, a surface inside another must be before it) for layout
    /// generation.
    pub fn commit(&mut self, mut transaction: Transaction) {
        // optimize all blocks
        let block_optimize_start = time::precise_time_s();
        for block in transaction.blocks.iter_mut() {
            pass::remove_dead_code(block);
        }
        println!(
            "Block MIR optimize took {}ms",
            (time::precise_time_s() - block_optimize_start) * 1000.
        );

        // group extractors and run basic surface optimizations
        let surface_optimize_start = time::precise_time_s();
        let surfaces: Vec<_> = transaction
            .surfaces
            .into_iter()
            .flat_map(|mut surface| {
                let mut new_surfaces = pass::group_extracted(&mut surface, self);

                pass::remove_dead_groups(&mut surface);

                // we know the new surfaces go inside the current one, so ensure they're after it in the final order
                new_surfaces.push(surface);
                new_surfaces
            })
            .collect();
        println!(
            "Surface MIR optimize took {}ms",
            (time::precise_time_s() - surface_optimize_start) * 1000.
        );

        let new_block_ids: Vec<_> = transaction.blocks.iter().map(|block| block.id.id).collect();
        let new_surface_ids: Vec<_> = surfaces.iter().map(|surface| surface.id.id).collect();

        // patch in the new blocks
        let block_layout_start = time::precise_time_s();
        for block in transaction.blocks {
            let layout = data_analyzer::build_block_layout(&self.context, &block, &self.target);
            let module = self.context
                .create_module(&format!("block.{}.{}", block.id.id, block.id.debug_name));
            self.blocks.insert(block.id.id, (block, layout, module));
        }
        println!(
            "Block layout took {}ms",
            (time::precise_time_s() - block_layout_start) * 1000.
        );

        // patch in the new surfaces
        let surface_layout_start = time::precise_time_s();
        for surface in surfaces {
            let layout = data_analyzer::build_surface_layout(self, &surface);
            let module = self.context.create_module(&format!(
                "surface.{}.{}",
                surface.id.id, surface.id.debug_name
            ));
            self.surfaces
                .insert(surface.id.id, (surface, layout, module));
        }
        println!(
            "Surface layout took {}ms",
            (time::precise_time_s() - surface_layout_start) * 1000.
        );

        // generate functions into new modules
        let block_codegen_start = time::precise_time_s();
        for block_id in &new_block_ids {
            let &(ref block, _, ref module) = self.blocks.get(block_id).unwrap();
            block::build_funcs(module, self, block);
        }
        println!(
            "Block codegen took {}ms",
            (time::precise_time_s() - block_codegen_start) * 1000.
        );

        let surface_codegen_start = time::precise_time_s();
        for surface_id in &new_surface_ids {
            let &(ref surface, _, ref module) = self.surfaces.get(surface_id).unwrap();
            surface::build_funcs(module, self, surface);
        }
        println!(
            "Surface codegen took {}ms",
            (time::precise_time_s() - surface_codegen_start) * 1000.
        );

        // optimize modules
        // this can go with the other passes, except we want timing info here
        let block_optimize_start = time::precise_time_s();
        for block_id in &new_block_ids {
            let &(_, _, ref module) = self.blocks.get(block_id).unwrap();
            self.optimizer.optimize_module(module);
        }
        println!(
            "Block codegen optimize took {}ms",
            (time::precise_time_s() - block_optimize_start) * 1000.
        );

        let surface_optimize_start = time::precise_time_s();
        for surface_id in &new_surface_ids {
            let &(_, _, ref module) = self.surfaces.get(surface_id).unwrap();
            self.optimizer.optimize_module(module);
        }
        println!(
            "Surface codegen optimize took {}ms",
            (time::precise_time_s() - surface_optimize_start) * 1000.
        );

        // remove orphaned objects
        self.garbage_collect();
    }

    /// Remove any objects that aren't referenced by others (and aren't the root).
    pub fn garbage_collect(&mut self) {}

    pub fn surface_module(&self, id: u64) -> Option<&Module> {
        match self.surfaces.get(&id) {
            Some(surface) => Some(&surface.2),
            None => None,
        }
    }

    pub fn block_module(&self, id: u64) -> Option<&Module> {
        match self.blocks.get(&id) {
            Some(block) => Some(&block.2),
            None => None,
        }
    }

    pub fn print_modules(&self) {
        for (_, &(ref block, _, ref module)) in &self.blocks {
            println!("Block {:?}", block.id);
            module.print_to_stderr();
        }
        for (_, &(ref surface, _, ref module)) in &self.surfaces {
            println!("Surface {:?}", surface.id);
            module.print_to_stderr();
        }
    }
}

impl ObjectCache for Runtime {
    fn context(&self) -> &Context {
        &self.context
    }

    fn target(&self) -> &TargetProperties {
        &self.target
    }

    fn surface_mir(&self, id: u64) -> Option<&Surface> {
        match self.surfaces.get(&id) {
            Some(surface) => Some(&surface.0),
            None => None,
        }
    }

    fn surface_layout(&self, id: u64) -> Option<&data_analyzer::SurfaceLayout> {
        match self.surfaces.get(&id) {
            Some(surface) => Some(&surface.1),
            None => None,
        }
    }

    fn block_mir(&self, id: u64) -> Option<&Block> {
        match self.blocks.get(&id) {
            Some(block) => Some(&block.0),
            None => None,
        }
    }

    fn block_layout(&self, id: u64) -> Option<&data_analyzer::BlockLayout> {
        match self.blocks.get(&id) {
            Some(block) => Some(&block.1),
            None => None,
        }
    }
}

impl IdAllocator for Runtime {
    fn alloc_id(&mut self) -> u64 {
        let take_id = self.next_id;
        self.next_id += 1;
        take_id
    }
}
