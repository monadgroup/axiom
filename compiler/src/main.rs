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

fn run_code(code: &str) {
    let mut stream = get_token_stream(code);

    let parse_start_time = time::precise_time_s();
    let ast = Parser::parse(&mut stream);
    println!("Parse took {}s", time::precise_time_s() - parse_start_time);

    let mir = ast.and_then(|ast| {
        let lower_start = time::precise_time_s();
        let block = lower_ast(mir::BlockId::new_with_id("test".to_string(), 0), &ast);
        println!("Lower took {}s", time::precise_time_s() - lower_start);
        block
    });

    match mir {
        Ok(mut block) => {
            remove_dead_code(&mut block);
            println!("{:#?}", block);

            let context = inkwell::context::Context::create();
            let module = context.create_module("test");
            let target = codegen::TargetProperties::new(true, false);

            let codegen_start = time::precise_time_s();
            codegen::intrinsics::build_intrinsics(&module);
            codegen::controls::build_funcs(&module, &target);
            codegen::functions::build_funcs(&module, &target);

            codegen::block::build_construct_func(&module, &block, &target);
            codegen::block::build_update_func(&module, &block, &target);
            codegen::block::build_destruct_func(&module, &block, &target);
            println!("Codegen took {}s", time::precise_time_s() - codegen_start);

            if let Err(err) = module.verify() {
                module.print_to_stderr();
                println!("{}", err.to_string());
            } else {
                optimize_module(&module);
                module.print_to_stderr();
            }
        }
        Err(err) => {
            let (text, pos) = err.formatted();
            println!(
                "Error {}:{} to {}:{} - {}",
                pos.0.line, pos.0.column, pos.1.line, pos.1.column, text
            )
        }
    }
}

fn do_repl() {
    println!("Enter code followed by two newlines...");
    let stdin = io::stdin();
    let mut input = "".to_string();
    for line in stdin.lock().lines() {
        let unwrapped = line.unwrap();
        input.push_str(&unwrapped);
        input.push_str("\n");

        if unwrapped.is_empty() {
            run_code(&input);
            return;
        }
    }
}

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
    let mut allocator = MIRContext::new();
    let source_id = BlockId::new("source1".to_string(), &mut allocator);
    let source_block = mir::Block::new(
        source_id.clone(),
        vec![mir::block::Control::new(
            "bla".to_string(),
            ControlType::AudioExtract,
            true,
            false,
        )],
        vec![],
    );
    allocator.register_block(source_block);

    let reader_id = BlockId::new("reader1".to_string(), &mut allocator);
    let reader_block = mir::Block::new(
        reader_id.clone(),
        vec![
            mir::block::Control::new("input".to_string(), ControlType::Audio, false, true),
            mir::block::Control::new("output".to_string(), ControlType::Audio, false, true),
        ],
        vec![],
    );
    allocator.register_block(reader_block);

    let groups = vec![
        ValueGroup::new(VarType::Num, ValueGroupSource::None),
        ValueGroup::new(VarType::new_array(VarType::Num), ValueGroupSource::None),
    ];
    let nodes = vec![
        Node::new(
            vec![ValueSocket::new(1, true, false, true)],
            NodeData::Custom(source_id.id),
        ),
        Node::new(
            vec![
                ValueSocket::new(1, false, true, false),
                ValueSocket::new(0, true, false, false),
            ],
            NodeData::Custom(reader_id.id),
        ),
    ];
    let surface_id = SurfaceId::new("test".to_string(), &mut allocator);
    let mut surface = Surface::new(surface_id.clone(), groups, nodes);

    let context = inkwell::context::Context::create();
    let module = context.create_module("test");
    let target = TargetProperties::new(true, false);

    let new_surfaces = group_extracted(&mut surface, &mut allocator);
    for new_surface in new_surfaces.into_iter() {
        println!(
            "Surface {:?}: {:#?}",
            new_surface.id,
            codegen::data_analyzer::build_surface_layout(
                &context,
                &allocator,
                &new_surface,
                &target
            )
        );
        allocator.register_surface(new_surface);
    }

    remove_dead_groups(&mut surface);
    println!(
        "Root surface {:?}: {:#?}",
        surface.id,
        codegen::data_analyzer::build_surface_layout(&context, &allocator, &surface, &target)
    );

    codegen::surface::build_funcs(&module, &allocator, &surface, &target);
    if let Err(e) = module.verify() {
        module.print_to_stderr();
        println!("{}", e.to_string());
    } else {
        optimize_module(&module);
        module.print_to_stderr();
    }

    /*let context = inkwell::context::Context::create();
    let module = context.create_module("test");
    let target = TargetProperties::new(true, false);

    let new_surfaces = group_extracted(&mut surface, &mut allocator);
    for new_surface in new_surfaces.into_iter() {
        println!("New surface: {:#?}", new_surface);
        println!("{:#?}", codegen::data_analyzer::build_surface_layout(&context, &allocator, &new_surface, &target));
        allocator.register_surface(new_surface);
    }

    println!("Root surface: {:#?}", surface);
    println!("{:#?}", codegen::data_analyzer::build_surface_layout(&context, &allocator, &surface, &target));
    allocator.register_surface(surface);

    codegen::surface::build_funcs(
        &module,
        &allocator,
        allocator.surface(&surface_id).unwrap(),
        &target,
    );

    if let Err(e) = module.verify() {
        module.print_to_stderr();
        println!("{}", e.to_string());
    } else {
        optimize_module(&module);
        module.print_to_stderr();
    }*/
}
