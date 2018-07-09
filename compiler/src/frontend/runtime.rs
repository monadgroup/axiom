use super::dependency_graph::DependencyGraph;
use super::jit::{Jit, JitKey};
use super::Transaction;
use codegen::{
    block, controls, converters, data_analyzer, functions, globals, intrinsics, root, surface,
    ObjectCache, Optimizer, TargetProperties,
};
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
use std::ptr;

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
const PORTALS_GLOBAL_NAME: &str = "maxim.runtime.portals";
const POINTERS_GLOBAL_NAME: &str = "maxim.runtime.pointers";

const CONSTRUCT_FUNC_NAME: &str = "maxim.runtime.construct";
const UPDATE_FUNC_NAME: &str = "maxim.runtime.update";
const DESTRUCT_FUNC_NAME: &str = "maxim.runtime.destruct";

#[derive(Debug)]
struct RuntimePointers {
    initialized_ptr: *mut c_void,
    scratch_ptr: *mut c_void,
    sockets_ptr: *mut c_void,
    portals_ptr: *mut c_void,
    pointers_ptr: *mut c_void,
    samplerate_ptr: *mut c_void,
    bpm_ptr: *mut c_void,
    construct: unsafe extern "C" fn(),
    update: unsafe extern "C" fn(),
    destruct: unsafe extern "C" fn(),
}

impl RuntimePointers {
    pub fn new(jit: &Jit) -> Self {
        let construct_address = jit.get_symbol_address(CONSTRUCT_FUNC_NAME) as usize;
        assert_ne!(construct_address, 0);

        let update_address = jit.get_symbol_address(UPDATE_FUNC_NAME) as usize;
        assert_ne!(update_address, 0);

        let destruct_address = jit.get_symbol_address(DESTRUCT_FUNC_NAME) as usize;
        assert_ne!(destruct_address, 0);

        let samplerate_ptr_address =
            jit.get_symbol_address(globals::SAMPLERATE_GLOBAL_NAME) as usize;
        assert_ne!(samplerate_ptr_address, 0);

        let bpm_ptr_address = jit.get_symbol_address(globals::BPM_GLOBAL_NAME) as usize;
        assert_ne!(bpm_ptr_address, 0);

        // pointers can be null when they're pointing to empty data
        let initialized_ptr_address = jit.get_symbol_address(INITIALIZED_GLOBAL_NAME) as usize;
        let scratch_ptr_address = jit.get_symbol_address(SCRATCH_GLOBAL_NAME) as usize;
        let sockets_ptr_address = jit.get_symbol_address(SOCKETS_GLOBAL_NAME) as usize;
        let portals_ptr_address = jit.get_symbol_address(PORTALS_GLOBAL_NAME) as usize;
        let pointers_ptr_address = jit.get_symbol_address(POINTERS_GLOBAL_NAME) as usize;

        RuntimePointers {
            initialized_ptr: initialized_ptr_address as *mut c_void,
            scratch_ptr: scratch_ptr_address as *mut c_void,
            sockets_ptr: sockets_ptr_address as *mut c_void,
            portals_ptr: portals_ptr_address as *mut c_void,
            pointers_ptr: pointers_ptr_address as *mut c_void,
            samplerate_ptr: samplerate_ptr_address as *mut c_void,
            bpm_ptr: bpm_ptr_address as *mut c_void,
            construct: unsafe { mem::transmute(construct_address) },
            update: unsafe { mem::transmute(update_address) },
            destruct: unsafe { mem::transmute(destruct_address) },
        }
    }
}

#[derive(Debug)]
pub struct Runtime<'a> {
    next_id: u64,
    context: Context,
    target: TargetProperties,
    pub optimizer: Optimizer,
    root: (Root, RuntimeModule),
    surfaces: HashMap<SurfaceRef, (Surface, data_analyzer::SurfaceLayout, RuntimeModule)>,
    blocks: HashMap<BlockRef, (Block, data_analyzer::BlockLayout, RuntimeModule)>,
    graph: DependencyGraph,
    jit: Option<&'a Jit>,
    pointers: Option<RuntimePointers>,
    bpm: f32,
    sample_rate: f32,
}

impl<'a> Runtime<'a> {
    pub fn new(target: TargetProperties, jit: Option<&'a Jit>) -> Self {
        let optimizer = Optimizer::new(&target);
        let context = Context::create();
        let root_module = Runtime::create_module(&context, &target, "root");

        // deploy the library to the JIT
        if let Some(ref jit) = jit {
            let library_module = Runtime::codegen_lib(&context, &target);
            optimizer.optimize_module(&library_module);
            jit.deploy(&library_module);
        }

        Runtime {
            next_id: 1,
            context,
            target,
            optimizer,
            root: (Root::new(Vec::new()), RuntimeModule::new(root_module, None)),
            surfaces: HashMap::new(),
            blocks: HashMap::new(),
            graph: DependencyGraph::new(),
            jit,
            pointers: None,
            bpm: 60.,
            sample_rate: 44100.,
        }
    }

    fn create_module(context: &Context, target: &TargetProperties, name: &str) -> Module {
        let module = context.create_module(name);
        module.set_target(&target.machine.get_triple().to_string_lossy());
        module.set_data_layout(&target.machine.get_data().get_data_layout());
        module
    }

    fn codegen_lib(context: &Context, target: &TargetProperties) -> Module {
        let module = Runtime::create_module(context, target, "lib");
        controls::build_funcs(&module, target);
        converters::build_funcs(&module);
        functions::build_funcs(&module, &target);
        intrinsics::build_intrinsics(&module);
        globals::build_globals(&module);
        module
    }

    fn optimize_blocks<'b>(&self, blocks: impl IntoIterator<Item = &'b mut Block>) {
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
            let module = Runtime::create_module(
                &self.context,
                &self.target,
                &format!("block.{}.{}", block.id.id, block.id.debug_name),
            );
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
            let module = Runtime::create_module(
                &self.context,
                &self.target,
                &format!("surface.{}.{}", surface.id.id, surface.id.debug_name),
            );
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
        let module = Runtime::create_module(&self.context, &self.target, "root");
        let initialized_global =
            root::build_initialized_global(&module, self, 0, INITIALIZED_GLOBAL_NAME);
        let scratch_global = root::build_scratch_global(&module, self, 0, SCRATCH_GLOBAL_NAME);
        let sockets_global =
            root::build_sockets_global(&module, root, SOCKETS_GLOBAL_NAME, PORTALS_GLOBAL_NAME);
        let pointers_global = root::build_pointers_global(
            &module,
            self,
            0,
            POINTERS_GLOBAL_NAME,
            initialized_global.as_pointer_value(),
            scratch_global.as_pointer_value(),
            sockets_global.sockets.as_pointer_value(),
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
        self.optimizer.optimize_module(&module);
        module
    }

    fn sort_surfaces(surfaces: &mut [Surface], graph: &DependencyGraph) {
        surfaces.sort_unstable_by(|a, b| {
            // if A is in B's depended_by list, A goes before
            // if A is in B's depends_on list, A goes after
            // otherwise, we don't know, so return equal

            let b_deps = graph.get_surface_deps(b.id.id).unwrap();
            if b_deps.depended_by.contains(&a.id.id) {
                Ordering::Greater
            } else if b_deps.depends_on_surfaces.contains(&a.id.id) {
                Ordering::Less
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

    fn patch_transaction(&mut self, transaction: Transaction) -> (Vec<BlockRef>, Vec<SurfaceRef>) {
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
        self.patch_in_surfaces(surfaces);
        if let Some(new_root) = transaction.root {
            self.root.0 = new_root;
        }

        // remove orphaned objects
        self.garbage_collect();

        (new_block_ids, new_surface_ids)
    }

    fn codegen_transaction(
        &mut self,
        new_block_ids: Vec<BlockRef>,
        new_surface_ids: Vec<SurfaceRef>,
    ) {
        self.codegen_blocks(&new_block_ids);
        self.codegen_surfaces(&new_surface_ids);
        let new_module = RuntimeModule::new(self.codegen_root(&self.root.0), None);

        if let Some(ref jit) = self.jit {
            Runtime::remove_module(jit, &self.root.1);
        }
        self.root.1 = new_module;

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

            Runtime::deploy_module(jit, &mut self.root.1);

            self.pointers = Some(RuntimePointers::new(jit));
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

        let (new_block_ids, new_surface_ids) = self.patch_transaction(transaction);
        self.codegen_transaction(new_block_ids, new_surface_ids);

        if let Some(ref pointers) = self.pointers {
            // re-set the BPM and sample rate
            Runtime::set_vector(pointers.bpm_ptr, self.bpm);
            Runtime::set_vector(pointers.samplerate_ptr, self.sample_rate);

            // run the new constructors
            unsafe {
                (pointers.construct)();
            }
        }
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

    pub unsafe fn get_portal_ptr(&self, portal_index: usize) -> *mut c_void {
        if let Some(ref pointers) = self.pointers {
            let portals_array = pointers.portals_ptr as *mut *mut c_void;
            *portals_array.offset(portal_index as isize)
        } else {
            ptr::null_mut()
        }
    }

    fn set_vector(ptr: *mut c_void, value: f32) {
        let vec_ptr = ptr as *mut (f32, f32);
        unsafe {
            (*vec_ptr).0 = value;
            (*vec_ptr).1 = value;
        }
    }

    pub fn set_bpm(&mut self, bpm: f32) {
        self.bpm = bpm;
        if let Some(ref pointers) = self.pointers {
            Runtime::set_vector(pointers.bpm_ptr, bpm);
        }
    }

    pub fn set_sample_rate(&mut self, sample_rate: f32) {
        self.sample_rate = sample_rate;
        if let Some(ref pointers) = self.pointers {
            Runtime::set_vector(pointers.samplerate_ptr, sample_rate);
        }
    }

    pub fn print_mir(&self) {
        println!(">> Begin MIR");
        println!("Blocks >>");
        for (_, &(ref block, _, _)) in &self.blocks {
            println!("{:#?}", block);
        }
        println!("Surfaces >>");
        for (_, &(ref surface, _, _)) in &self.surfaces {
            println!("{:#?}", surface);
        }
        println!("Root: {:#?}", self.root.0);
        println!("<< End MIR");
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

        println!("Root");
        self.root.1.module.print_to_stderr();
    }
}

impl<'a> ObjectCache for Runtime<'a> {
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

impl<'a> IdAllocator for Runtime<'a> {
    fn alloc_id(&mut self) -> u64 {
        let take_id = self.next_id;
        self.next_id += 1;
        take_id
    }
}

impl<'a> Drop for Runtime<'a> {
    fn drop(&mut self) {
        if let Some(ref pointers) = self.pointers {
            unsafe {
                (pointers.destruct)();
            }
        }
    }
}
