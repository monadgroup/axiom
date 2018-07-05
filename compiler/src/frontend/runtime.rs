use super::dependency_graph::DependencyGraph;
use super::jit::{Jit, JitKey};
use super::Transaction;
use codegen::{block, data_analyzer, root, surface, ObjectCache, Optimizer, TargetProperties};
use inkwell::context::Context;
use inkwell::module::Module;
use mir::{Block, BlockRef, IdAllocator, Root, Surface, SurfaceRef};
use pass;
use std::cmp::Ordering;
use std::collections::{HashMap, HashSet, VecDeque};
use std::iter;
use std::iter::FromIterator;
use std::mem;
use std::os::raw::c_void;

#[derive(Debug)]
struct RuntimeModule {
    module: Module,
    key: Option<JitKey>,
}

impl RuntimeModule {
    pub fn new(module: Module, key: Option<JitKey>) -> Self {
        RuntimeModule { module, key }
    }
}

const INITIALIZED_GLOBAL_NAME: &str = "maxim.runtime.initialized";
const SCRATCH_GLOBAL_NAME: &str = "maxim.runtime.scratch";
const SOCKETS_GLOBAL_NAME: &str = "maxim.runtime.sockets";
const POINTERS_GLOBAL_NAME: &str = "maxim.runtime.pointers";

const CONSTRUCT_FUNC_NAME: &str = "maxim.runtime.construct";
const UPDATE_FUNC_NAME: &str = "maxim.update.construct";
const DESTRUCT_FUNC_NAME: &str = "maxim.destruct.construct";

#[derive(Debug)]
struct RuntimePointers {
    initialized_ptr: *mut c_void,
    scratch_ptr: *mut c_void,
    sockets_ptr: *mut c_void,
    pointers_ptr: *mut c_void,
    construct: unsafe extern "C" fn(),
    update: unsafe extern "C" fn(),
    destruct: unsafe extern "C" fn(),
}

impl RuntimePointers {
    pub fn new(jit: &Jit) -> Self {
        RuntimePointers {
            initialized_ptr: unsafe {
                mem::transmute(jit.get_symbol_address(INITIALIZED_GLOBAL_NAME).unwrap() as usize)
            },
            scratch_ptr: unsafe {
                mem::transmute(jit.get_symbol_address(SCRATCH_GLOBAL_NAME).unwrap() as usize)
            },
            sockets_ptr: unsafe {
                mem::transmute(jit.get_symbol_address(SOCKETS_GLOBAL_NAME).unwrap() as usize)
            },
            pointers_ptr: unsafe {
                mem::transmute(jit.get_symbol_address(POINTERS_GLOBAL_NAME).unwrap() as usize)
            },
            construct: unsafe {
                mem::transmute(jit.get_symbol_address(CONSTRUCT_FUNC_NAME).unwrap() as usize)
            },
            update: unsafe {
                mem::transmute(jit.get_symbol_address(UPDATE_FUNC_NAME).unwrap() as usize)
            },
            destruct: unsafe {
                mem::transmute(jit.get_symbol_address(DESTRUCT_FUNC_NAME).unwrap() as usize)
            },
        }
    }
}

#[derive(Debug)]
pub struct Runtime {
    next_id: u64,
    context: Context,
    target: TargetProperties,
    pub optimizer: Optimizer,
    root: (Root, RuntimeModule),
    surfaces: HashMap<SurfaceRef, (Surface, data_analyzer::SurfaceLayout, RuntimeModule)>,
    blocks: HashMap<BlockRef, (Block, data_analyzer::BlockLayout, RuntimeModule)>,
    graph: DependencyGraph,
    jit: Option<Jit>,
    pointers: Option<RuntimePointers>,
}

impl Runtime {
    pub fn new(target: TargetProperties, jit: Option<Jit>) -> Self {
        let optimizer = Optimizer::new(&target);
        Runtime {
            next_id: 1,
            context: Context::create(),
            target,
            optimizer,
            root: (
                Root::new(Vec::new()),
                RuntimeModule::new(Module::create("root"), None),
            ),
            surfaces: HashMap::new(),
            blocks: HashMap::new(),
            graph: DependencyGraph::new(),
            jit,
            pointers: None,
        }
    }

    fn optimize_blocks<'a>(&self, blocks: impl IntoIterator<Item = &'a mut Block>) {
        for block in blocks.into_iter() {
            pass::remove_dead_code(block);
        }
    }

    fn optimize_surfaces(&mut self, surfaces: impl IntoIterator<Item = Surface>) -> Vec<Surface> {
        surfaces
            .into_iter()
            .flat_map(|mut surface| {
                let new_surfaces = pass::group_extracted(&mut surface, self);
                pass::remove_dead_groups(&mut surface);
                new_surfaces.into_iter().chain(iter::once(surface))
            })
            .collect()
    }

    fn patch_in_blocks(&mut self, blocks: Vec<Block>) {
        for block in blocks {
            let layout = data_analyzer::build_block_layout(&self.context, &block, &self.target);
            let module = self.context
                .create_module(&format!("block.{}.{}", block.id.id, block.id.debug_name));
            if let Some(old_block) = self.blocks.insert(
                block.id.id,
                (block, layout, RuntimeModule::new(module, None)),
            ) {
                if let Some(ref jit) = self.jit {
                    Runtime::remove_module(jit, &old_block.2);
                }
            }
        }
    }

    fn patch_in_surfaces(&mut self, surfaces: Vec<Surface>) {
        for surface in surfaces {
            let layout = data_analyzer::build_surface_layout(self, &surface);
            let module = self.context.create_module(&format!(
                "surface.{}.{}",
                surface.id.id, surface.id.debug_name
            ));
            if let Some(old_block) = self.surfaces.insert(
                surface.id.id,
                (surface, layout, RuntimeModule::new(module, None)),
            ) {
                if let Some(ref jit) = self.jit {
                    Runtime::remove_module(jit, &old_block.2);
                }
            }
        }
    }

    fn codegen_blocks(&self, block_ids: &[BlockRef]) {
        for block_id in block_ids {
            let (ref block, _, ref module) = self.blocks[block_id];
            block::build_funcs(&module.module, self, block);
            self.optimizer.optimize_module(&module.module);
        }
    }

    fn codegen_surfaces(&self, surface_ids: &[SurfaceRef]) {
        for surface_id in surface_ids {
            let (ref surface, _, ref module) = self.surfaces[surface_id];
            surface::build_funcs(&module.module, self, surface);
            self.optimizer.optimize_module(&module.module);
        }
    }

    fn codegen_root(&self, root: &Root) -> Module {
        let module = Module::create("root");
        let initialized_global =
            root::build_initialized_global(&module, self, 0, INITIALIZED_GLOBAL_NAME);
        let scratch_global = root::build_scratch_global(&module, self, 0, SCRATCH_GLOBAL_NAME);
        let sockets_global = root::build_sockets_global(&module, root, SOCKETS_GLOBAL_NAME);
        let pointers_global = root::build_pointers_global(
            &module,
            self,
            0,
            POINTERS_GLOBAL_NAME,
            initialized_global.as_pointer_value(),
            scratch_global.as_pointer_value(),
            sockets_global.as_pointer_value(),
        );
        root::build_funcs(
            &module,
            self,
            0,
            CONSTRUCT_FUNC_NAME,
            UPDATE_FUNC_NAME,
            DESTRUCT_FUNC_NAME,
            scratch_global.as_pointer_value(),
            pointers_global.as_pointer_value(),
        );
        module
    }

    fn sort_surfaces(surfaces: &mut [Surface], graph: &DependencyGraph) {
        surfaces.sort_unstable_by(|a, b| {
            // if A is in B's depended_by list, A goes before
            // if A is in B's depends_on list, A goes after
            // otherwise, we don't know, so return equal

            let b_deps = graph.get_surface_deps(b.id.id).unwrap();
            if b_deps.depended_by.contains(&a.id.id) {
                Ordering::Less
            } else if b_deps.depends_on_surfaces.contains(&a.id.id) {
                Ordering::Greater
            } else {
                Ordering::Equal
            }
        });
    }

    fn get_deployable_surfaces(
        graph: &DependencyGraph,
        blocks: &[BlockRef],
        surfaces: &[SurfaceRef],
    ) -> Vec<SurfaceRef> {
        let mut required_surfaces = HashSet::new();
        let mut deploy_surfaces = VecDeque::new();

        // add the surfaces we have
        for &surface in surfaces {
            deploy_surfaces.push_back(surface);
            required_surfaces.insert(surface);
        }

        // add the surfaces each block depends on
        for &block_ref in blocks {
            let dependent_surfaces = &graph.get_block_deps(block_ref).unwrap().depended_by;
            for dependent_surface in dependent_surfaces {
                if required_surfaces.contains(dependent_surface) {
                    continue;
                }

                deploy_surfaces.push_back(*dependent_surface);
                required_surfaces.insert(*dependent_surface);
            }
        }

        // walk up the dependency chain
        while let Some(walk_surface) = deploy_surfaces.pop_front() {
            let dependent_surfaces = &graph.get_surface_deps(walk_surface).unwrap().depended_by;
            for dependent_surface in dependent_surfaces {
                if required_surfaces.contains(dependent_surface) {
                    continue;
                }

                deploy_surfaces.push_back(*dependent_surface);
                required_surfaces.insert(*dependent_surface);
            }
        }

        // todo: do we need to sort surfaces in dependency order for the JIT linking?
        Vec::from_iter(required_surfaces.into_iter())
    }

    fn deploy_module(jit: &Jit, module: &mut RuntimeModule) {
        let key = jit.deploy(&module.module);
        module.key = Some(key);
    }

    fn remove_module(jit: &Jit, module: &RuntimeModule) {
        if let Some(key) = module.key {
            jit.remove(key);
        }
    }

    pub fn commit(&mut self, transaction: Transaction) {
        // if the transaction is empty, early exit
        if transaction.surfaces.is_empty()
            && transaction.blocks.is_empty()
            && transaction.root.is_none()
        {
            return;
        }

        // run destructors on the old data before beginning
        if let Some(ref pointers) = self.pointers {
            unsafe {
                (pointers.destruct)();
            }
        }

        let mut surfaces =
            self.optimize_surfaces(transaction.surfaces.into_iter().map(|(_, surface)| surface));
        let mut blocks: Vec<_> = transaction
            .blocks
            .into_iter()
            .map(|(_, block)| block)
            .collect();
        self.optimize_blocks(blocks.iter_mut());

        // add the new surfaces to the dependency graph
        for surface in &surfaces {
            self.graph.generate_surface(surface);
        }

        // sort the surfaces in dependency order - this is important for when we generate types,
        // and must be done after generating the graph (since it depends on the graph!)
        Runtime::sort_surfaces(&mut surfaces, &self.graph);

        // keep track of IDs of the new surfaces and blocks, for after we move them into the runtime
        let new_block_ids: Vec<_> = blocks.iter().map(|block| block.id.id).collect();
        let new_surface_ids: Vec<_> = surfaces.iter().map(|surface| surface.id.id).collect();

        self.patch_in_blocks(blocks);
        self.codegen_blocks(&new_block_ids);
        self.patch_in_surfaces(surfaces);
        self.codegen_surfaces(&new_surface_ids);

        // build the new root module
        if let Some(new_root) = transaction.root {
            self.root.0 = new_root;
        }
        let mut new_module = RuntimeModule::new(self.codegen_root(&self.root.0), None);

        // deploy all modules and their parents
        if let Some(ref jit) = self.jit {
            for block in &new_block_ids {
                Runtime::deploy_module(jit, &mut self.blocks.get_mut(block).unwrap().2);
            }
            for surface in
                Runtime::get_deployable_surfaces(&self.graph, &new_block_ids, &new_surface_ids)
            {
                Runtime::deploy_module(jit, &mut self.surfaces.get_mut(&surface).unwrap().2);
            }

            Runtime::remove_module(jit, &self.root.1);
            Runtime::deploy_module(jit, &mut new_module);

            // find the new pointers and run the constructors on the new data
            let pointers = RuntimePointers::new(jit);
            unsafe {
                (pointers.construct)();
            }
            self.pointers = Some(pointers);
        }
        self.root.1 = new_module;

        // remove orphaned objects
        self.garbage_collect();
    }

    /// Remove any objects that aren't referenced by others (and aren't the root).
    pub fn garbage_collect(&mut self) {
        self.graph.garbage_collect();

        let graph = &self.graph;
        let jit = &self.jit;

        // we can now remove any objects that don't exist in the graph
        self.surfaces.retain(|&key, &mut (_, _, ref module)| {
            if graph.get_surface_deps(key).is_some() {
                true
            } else {
                if let &Some(ref jit) = jit {
                    Runtime::remove_module(jit, module);
                }
                false
            }
        });
        self.blocks.retain(|&key, &mut (_, _, ref module)| {
            if graph.get_block_deps(key).is_some() {
                true
            } else {
                if let &Some(ref jit) = jit {
                    Runtime::remove_module(jit, module);
                }
                false
            }
        });
    }

    pub unsafe fn run_update(&self) {
        if let Some(ref pointers) = self.pointers {
            (pointers.update)();
        }
    }

    pub fn print_modules(&self) {
        for (_, &(ref block, _, ref module)) in &self.blocks {
            println!("Block {:?}", block.id);
            module.module.print_to_stderr();
        }
        for (_, &(ref surface, _, ref module)) in &self.surfaces {
            println!("Surface {:?}", surface.id);
            module.module.print_to_stderr();
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

    fn surface_mir(&self, id: SurfaceRef) -> Option<&Surface> {
        match self.surfaces.get(&id) {
            Some(surface) => Some(&surface.0),
            None => None,
        }
    }

    fn surface_layout(&self, id: SurfaceRef) -> Option<&data_analyzer::SurfaceLayout> {
        match self.surfaces.get(&id) {
            Some(surface) => Some(&surface.1),
            None => None,
        }
    }

    fn block_mir(&self, id: BlockRef) -> Option<&Block> {
        match self.blocks.get(&id) {
            Some(block) => Some(&block.0),
            None => None,
        }
    }

    fn block_layout(&self, id: BlockRef) -> Option<&data_analyzer::BlockLayout> {
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
