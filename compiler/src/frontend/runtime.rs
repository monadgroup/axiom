use super::dependency_graph::DependencyGraph;
use super::jit::{Jit, JitKey};
use super::Transaction;
use codegen::{
    block, controls, converters, data_analyzer, editor, functions, globals, intrinsics, root,
    surface, values, ObjectCache, Optimizer, TargetProperties,
};
use inkwell::context::Context;
use inkwell::module::Module;
use mir::{Block, BlockRef, IdAllocator, InternalNodeRef, Root, Surface, SurfaceRef};
use pass;
use std::cmp::Ordering;
use std::collections::hash_map::Entry;
use std::collections::{HashMap, HashSet, VecDeque};
use std::iter;
use std::iter::FromIterator;
use std::mem;
use std::os::raw::c_void;
use std::ptr;
use std::time::{Duration, Instant};

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

const CONVERT_NUM_FUNC_NAME: &str = "maxim.editor.convert_num";

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
    convert_num: unsafe extern "C" fn(*mut c_void, i8, *const c_void),
}

impl RuntimePointers {
    pub fn new(jit: &Jit) -> Self {
        let construct_address = jit.get_symbol_address(CONSTRUCT_FUNC_NAME) as usize;
        assert_ne!(construct_address, 0);

        let update_address = jit.get_symbol_address(UPDATE_FUNC_NAME) as usize;
        assert_ne!(update_address, 0);

        let destruct_address = jit.get_symbol_address(DESTRUCT_FUNC_NAME) as usize;
        assert_ne!(destruct_address, 0);

        let convert_num_address = jit.get_symbol_address(CONVERT_NUM_FUNC_NAME) as usize;
        assert_ne!(convert_num_address, 0);

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
            convert_num: unsafe { mem::transmute(convert_num_address) },
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
    surface_mirs: HashMap<SurfaceRef, Surface>,
    surface_layouts: HashMap<SurfaceRef, data_analyzer::SurfaceLayout>,
    surface_modules: HashMap<SurfaceRef, RuntimeModule>,
    block_mirs: HashMap<BlockRef, Block>,
    block_layouts: HashMap<BlockRef, data_analyzer::BlockLayout>,
    block_modules: HashMap<BlockRef, RuntimeModule>,
    graph: DependencyGraph,
    jit: Jit,
    pointers: Option<RuntimePointers>,
    bpm: f32,
    sample_rate: f32,
}

impl Runtime {
    pub fn new(target: TargetProperties) -> Self {
        let optimizer = Optimizer::new(&target);
        let context = Context::create();
        let root_module = Runtime::create_module(&context, &target, "root");
        let jit = Jit::new();

        // deploy the library to the JIT
        let library_module = Runtime::codegen_lib(&context, &target);
        optimizer.optimize_module(&library_module);
        jit.deploy(&library_module);

        Runtime {
            next_id: 1,
            context,
            target,
            optimizer,
            root: (Root::new(Vec::new()), RuntimeModule::new(root_module, None)),
            surface_mirs: HashMap::new(),
            surface_layouts: HashMap::new(),
            surface_modules: HashMap::new(),
            block_mirs: HashMap::new(),
            block_layouts: HashMap::new(),
            block_modules: HashMap::new(),
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
        values::MidiValue::initialize(&module, context);
        editor::build_convert_num_func(&module, &target, CONVERT_NUM_FUNC_NAME);
        module
    }

    fn sort_surfaces(surfaces: &mut [SurfaceRef], graph: &DependencyGraph) {
        surfaces.sort_unstable_by(|&a, &b| {
            // if A is in B's depended_by list, A goes before
            // if A is in B's depends_on list, A goes after
            // otherwise, we don't know, so return equal

            if graph.is_ancestor_of_surface(b, a) {
                Ordering::Greater
            } else if graph.is_descendent_of_surface(b, a) {
                Ordering::Less
            } else {
                Ordering::Equal
            }
        });
    }

    fn get_affected_surfaces(
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

        Vec::from_iter(required_surfaces.into_iter())
    }

    fn deploy_module(jit: &Jit, module: &mut RuntimeModule) {
        // if the module already has a key, remove it
        if let Some(key) = module.key {
            jit.remove(key);
        }
        let key = jit.deploy(&module.module);
        module.key = Some(key);
    }

    fn remove_module(jit: &Jit, module: &mut RuntimeModule) {
        if let Some(key) = module.key {
            jit.remove(key);
            module.key = None;
        }
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
            .map(|mut surface| {
                pass::order_nodes(&mut surface);
                surface
            })
            .collect()
    }

    fn patch_in_blocks(&mut self, blocks: Vec<Block>) {
        for block in blocks {
            let id = block.id.id;
            self.block_layouts.insert(
                id,
                data_analyzer::build_block_layout(&self.context, &block, &self.target),
            );
            self.block_mirs.insert(id, block);
        }
    }

    fn patch_in_surfaces(&mut self, surfaces: Vec<Surface>, build_layout_surfaces: &[u64]) {
        for surface in surfaces {
            let id = surface.id.id;
            self.surface_mirs.insert(id, surface);
        }

        // rebuild layouts for the flagged surfaces
        for build_layout_surface in build_layout_surfaces {
            let surface = &self.surface_mirs[build_layout_surface];
            let layout = data_analyzer::build_surface_layout(self, surface);
            self.surface_layouts.insert(*build_layout_surface, layout);
        }
    }

    fn patch_transaction(&mut self, transaction: Transaction) -> (Vec<BlockRef>, Vec<SurfaceRef>) {
        let surfaces =
            self.optimize_surfaces(transaction.surfaces.into_iter().map(|(_, surface)| surface));
        let mut blocks: Vec<_> = transaction
            .blocks
            .into_iter()
            .map(|(_, block)| block)
            .collect();
        self.optimize_blocks(blocks.iter_mut());

        // add the new surfaces to the dependency graph and remove old ones
        for surface in &surfaces {
            self.graph.generate_surface(surface);
        }
        self.graph.garbage_collect();

        let new_block_ids: Vec<_> = blocks.iter().map(|block| block.id.id).collect();
        let new_surface_ids: Vec<_> = surfaces.iter().map(|surface| surface.id.id).collect();

        // Build a list of affected surfaces (i.e surfaces whose layouts may have changed) to
        // recalculate layouts. This list must be sorted in dependency order, since layout
        // calculation depends on layouts of surfaces inside.
        let mut affected_surfaces =
            Runtime::get_affected_surfaces(&self.graph, &new_block_ids, &new_surface_ids);
        Runtime::sort_surfaces(&mut affected_surfaces, &self.graph);

        self.patch_in_blocks(blocks);
        self.patch_in_surfaces(surfaces, &affected_surfaces);
        if let Some(new_root) = transaction.root {
            self.root.0 = new_root;
        }

        // remove orphaned objects
        self.garbage_collect();

        (new_block_ids, affected_surfaces)
    }

    fn codegen_blocks(&mut self, block_ids: &[BlockRef]) {
        for &block_id in block_ids {
            let block = &self.block_mirs[&block_id];

            let module_id = if let Entry::Occupied(old_module) = self.block_modules.entry(block_id)
            {
                old_module.get().key
            } else {
                None
            };

            let module = RuntimeModule::new(
                Runtime::create_module(
                    &self.context,
                    &self.target,
                    &format!("block.{}.{}", block.id.id, block.id.debug_name),
                ),
                module_id,
            );
            block::build_funcs(&module.module, self, block);
            self.optimizer.optimize_module(&module.module);
            self.block_modules.insert(block_id, module);
        }
    }

    fn codegen_surfaces(&mut self, surface_ids: &[SurfaceRef]) {
        for &surface_id in surface_ids {
            let surface = &self.surface_mirs[&surface_id];

            let module_id =
                if let Entry::Occupied(old_module) = self.surface_modules.entry(surface_id) {
                    old_module.get().key
                } else {
                    None
                };

            let module = RuntimeModule::new(
                Runtime::create_module(
                    &self.context,
                    &self.target,
                    &format!("surface.{}.{}", surface.id.id, surface.id.debug_name),
                ),
                module_id,
            );
            surface::build_funcs(&module.module, self, surface);
            self.optimizer.optimize_module(&module.module);
            self.surface_modules.insert(surface_id, module);
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
            pointers_global.as_pointer_value(),
        );
        self.optimizer.optimize_module(&module);
        module
    }

    fn codegen_transaction(
        &mut self,
        new_block_ids: &[BlockRef],
        affected_surfaces: &[SurfaceRef],
    ) {
        self.codegen_blocks(new_block_ids);
        self.codegen_surfaces(affected_surfaces);

        self.root.1.module = self.codegen_root(&self.root.0);
    }

    fn deploy_transaction(&mut self, block_ids: &[BlockRef], affected_surfaces: &[SurfaceRef]) {
        for block in block_ids {
            Runtime::deploy_module(&self.jit, self.block_modules.get_mut(block).unwrap());
        }
        for surface in affected_surfaces {
            Runtime::deploy_module(&self.jit, self.surface_modules.get_mut(surface).unwrap());
        }

        Runtime::deploy_module(&self.jit, &mut self.root.1);
        self.pointers = Some(RuntimePointers::new(&self.jit));
    }

    pub fn commit(&mut self, transaction: Transaction) {
        // if the transaction is empty, early exit
        if transaction.surfaces.is_empty()
            && transaction.blocks.is_empty()
            && transaction.root.is_none()
        {
            return;
        }

        // run destructors on old data before beginning
        if let Some(ref pointers) = self.pointers {
            unsafe {
                (pointers.destruct)();
            }
        }

        println!("{:#?}", transaction);

        let patch_start = Instant::now();
        let (new_block_ids, affected_surfaces) = self.patch_transaction(transaction);
        println!(
            "Patch took {}s",
            precise_duration_seconds(&patch_start.elapsed())
        );

        let codegen_start = Instant::now();
        self.codegen_transaction(&new_block_ids, &affected_surfaces);
        println!(
            "Codegen took {}s",
            precise_duration_seconds(&codegen_start.elapsed())
        );

        let deploy_start = Instant::now();
        self.deploy_transaction(&new_block_ids, &affected_surfaces);
        println!(
            "Deploy took {}s",
            precise_duration_seconds(&deploy_start.elapsed())
        );

        if let Some(ref pointers) = self.pointers {
            // reset the BPM and sample rate
            Runtime::set_vector(pointers.bpm_ptr, self.bpm);
            Runtime::set_vector(pointers.samplerate_ptr, self.sample_rate);

            // run the new constructor
            unsafe {
                (pointers.construct)();
            }
        }
    }

    /// Remove any objects that aren't referenced by others (and aren't the root).
    pub fn garbage_collect(&mut self) {
        let graph = &self.graph;
        let surface_mirs = &mut self.surface_mirs;
        let surface_layouts = &mut self.surface_layouts;
        let block_mirs = &mut self.block_mirs;
        let block_layouts = &mut self.block_layouts;
        let jit = &self.jit;

        // we can now remove any objects that don't exist in the graph
        self.surface_modules.retain(|&key, module| {
            if graph.get_surface_deps(key).is_some() {
                true
            } else {
                surface_mirs.remove(&key);
                surface_layouts.remove(&key);
                Runtime::remove_module(jit, module);
                false
            }
        });
        self.block_modules.retain(|&key, module| {
            if graph.get_block_deps(key).is_some() {
                true
            } else {
                block_mirs.remove(&key);
                block_layouts.remove(&key);
                Runtime::remove_module(jit, module);
                false
            }
        });
    }

    pub unsafe fn run_update(&self) {
        if let Some(ref pointers) = self.pointers {
            (pointers.update)();
        }
    }

    pub fn get_root_ptr(&self) -> *mut c_void {
        if let Some(ref pointers) = self.pointers {
            pointers.pointers_ptr
        } else {
            ptr::null_mut()
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

    pub fn get_bpm(&self) -> f32 {
        self.bpm
    }

    pub fn set_sample_rate(&mut self, sample_rate: f32) {
        self.sample_rate = sample_rate;
        if let Some(ref pointers) = self.pointers {
            Runtime::set_vector(pointers.samplerate_ptr, sample_rate);
        }
    }

    pub fn get_sample_rate(&self) -> f32 {
        self.sample_rate
    }

    pub fn is_node_extracted(&self, surface: SurfaceRef, node: usize) -> bool {
        let surface_mir = self.surface_mir(surface).unwrap();
        let node_inner = surface_mir.source_map.map_to_internal(node);

        if let InternalNodeRef::Surface(_, _) = node_inner {
            true
        } else {
            false
        }
    }

    pub fn convert_num(&self, result: *mut c_void, target_form: i8, num: *const c_void) {
        if let Some(ref pointers) = self.pointers {
            unsafe { (pointers.convert_num)(result, target_form, num) }
        }
    }

    pub fn print_mir(&self) {
        println!(">> Begin MIR");
        println!("Blocks >>");
        for block in self.block_mirs.values() {
            println!("{:#?}", block);
        }
        println!("Surfaces >>");
        for surface in self.surface_mirs.values() {
            println!("{:#?}", surface);
        }
        println!("Root: {:#?}", self.root.0);
        println!("<< End MIR");
    }

    pub fn print_modules(&self) {
        for module in self.block_modules.values() {
            module.module.print_to_stderr();
        }
        for module in self.surface_modules.values() {
            module.module.print_to_stderr();
        }
        self.root.1.module.print_to_stderr();
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
        self.surface_mirs.get(&id)
    }

    fn surface_layout(&self, id: SurfaceRef) -> Option<&data_analyzer::SurfaceLayout> {
        self.surface_layouts.get(&id)
    }

    fn block_mir(&self, id: BlockRef) -> Option<&Block> {
        self.block_mirs.get(&id)
    }

    fn block_layout(&self, id: BlockRef) -> Option<&data_analyzer::BlockLayout> {
        self.block_layouts.get(&id)
    }
}

impl IdAllocator for Runtime {
    fn alloc_id(&mut self) -> u64 {
        let take_id = self.next_id;
        self.next_id += 1;
        take_id
    }
}

impl Drop for Runtime {
    fn drop(&mut self) {
        if let Some(ref pointers) = self.pointers {
            unsafe {
                (pointers.destruct)();
            }
        }
    }
}

fn precise_duration_seconds(duration: &Duration) -> f64 {
    duration.as_secs() as f64 + duration.subsec_nanos() as f64 / 1_000_000_000.
}
