use std::io;
use std::io::BufRead;

extern crate divrem;
extern crate inkwell;
extern crate ordered_float;
extern crate regex;
extern crate time;

#[macro_use]
extern crate lazy_static;

mod ast;
mod codegen;
mod compile_error;
mod frontend;
mod mir;
mod parser;
mod pass;
mod util;

pub use self::ast::*;
pub use self::codegen::*;
pub use self::compile_error::{CompileError, CompileResult};
pub use self::mir::*;
pub use self::parser::*;
pub use self::pass::*;
pub use self::util::*;

pub use frontend::c_api::{
    maxim_allocate_id, maxim_build_block, maxim_build_custom_node, maxim_build_group_node,
    maxim_build_surface, maxim_build_value_group, maxim_build_value_socket, maxim_commit,
    maxim_compile_block, maxim_constant_num, maxim_constant_tuple, maxim_create_runtime,
    maxim_create_transaction, maxim_destroy_error, maxim_destroy_runtime,
    maxim_valuegroupsource_default, maxim_valuegroupsource_none, maxim_valuegroupsource_socket,
    maxim_vararg_array, maxim_vartype_midi, maxim_vartype_num, maxim_vartype_tuple,
};

struct ModuleFunctionIterator<'a> {
    module: &'a inkwell::module::Module,
    next_func: Option<inkwell::values::FunctionValue>,
}

impl<'a> ModuleFunctionIterator<'a> {
    fn new(module: &'a inkwell::module::Module) -> ModuleFunctionIterator<'a> {
        ModuleFunctionIterator {
            module,
            next_func: module.get_first_function(),
        }
    }
}

impl<'a> Iterator for ModuleFunctionIterator<'a> {
    type Item = inkwell::values::FunctionValue;

    fn next(&mut self) -> Option<inkwell::values::FunctionValue> {
        match self.next_func {
            Some(func) => {
                self.next_func = func.get_next_function();
                Some(func)
            }
            None => None,
        }
    }
}

fn optimize_module(module: &inkwell::module::Module) {
    let pass_manager_builder = inkwell::passes::PassManagerBuilder::create();
    pass_manager_builder.set_optimization_level(inkwell::OptimizationLevel::Aggressive);
    pass_manager_builder.set_size_level(0);

    // threshold for -O3, see http://llvm.org/doxygen/InlineCost_8h_source.html#l00041
    pass_manager_builder.set_inliner_with_threshold(250);

    let module_pass = inkwell::passes::PassManager::create_for_module();
    pass_manager_builder.populate_module_pass_manager(&module_pass);

    let func_pass = inkwell::passes::PassManager::create_for_function(&module);
    pass_manager_builder.populate_function_pass_manager(&func_pass);

    func_pass.initialize();
    let func_iterator = ModuleFunctionIterator::new(&module);
    for func in func_iterator {
        func_pass.run_on_function(&func);
    }
    module_pass.initialize();
    module_pass.run_on_module(&module);
}

fn main() {
    // build a basic MIR
    let target = codegen::TargetProperties::new(true, false);
    let mut runtime = frontend::Runtime::new(target);

    let source_id = runtime.alloc_id();
    let source_block = mir::Block::new(
        BlockId::new_with_id("source1".to_string(), source_id),
        vec![mir::block::Control::new(
            "bla".to_string(),
            ControlType::AudioExtract,
            true,
            false,
        )],
        vec![],
    );

    let reader_id = runtime.alloc_id();
    let reader_block = mir::Block::new(
        BlockId::new_with_id("reader1".to_string(), reader_id),
        vec![
            mir::block::Control::new("input".to_string(), ControlType::Audio, false, true),
            mir::block::Control::new("output".to_string(), ControlType::Audio, false, true),
        ],
        vec![],
    );

    let surface_id = 0;
    let surface = mir::Surface::new(
        SurfaceId::new_with_id("surface".to_string(), surface_id),
        vec![
            ValueGroup::new(VarType::Num, ValueGroupSource::None),
            ValueGroup::new(VarType::new_array(VarType::Num), ValueGroupSource::None),
        ],
        vec![
            Node::new(
                vec![ValueSocket::new(1, true, false, true)],
                NodeData::Custom(source_id),
            ),
            Node::new(
                vec![
                    ValueSocket::new(1, false, true, false),
                    ValueSocket::new(0, true, false, false),
                ],
                NodeData::Custom(reader_id),
            ),
        ],
    );

    let transaction = frontend::Transaction::new(vec![surface], vec![source_block, reader_block]);

    let commit_start_time = time::precise_time_s();
    runtime.commit(transaction);
    println!(
        "Commit took {}ms",
        (time::precise_time_s() - commit_start_time) * 1000.
    );

    runtime.print_modules();

    let root_module = runtime.context().create_module("root");
    let initialized_global =
        codegen::root::build_initialized_global(&root_module, &runtime, surface_id, "initialized");
    let scratch_global =
        codegen::root::build_scratch_global(&root_module, &runtime, surface_id, "scratch");
    let pointers_global = codegen::root::build_pointers_global(
        &root_module,
        &runtime,
        surface_id,
        "pointers",
        initialized_global.as_pointer_value(),
        scratch_global.as_pointer_value(),
        runtime
            .context()
            .i8_type()
            .ptr_type(inkwell::AddressSpace::Generic)
            .get_undef(),
    );
    codegen::root::build_funcs(
        &root_module,
        &runtime,
        surface_id,
        "maxim_construct",
        "maxim_update",
        "maxim_destruct",
        scratch_global.as_pointer_value(),
        pointers_global.as_pointer_value(),
    );
    root_module.verify().unwrap();
    runtime.optimizer.optimize_module(&root_module);
    root_module.print_to_stderr();
}
