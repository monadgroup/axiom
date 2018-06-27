use std::io;
use std::io::BufRead;

extern crate inkwell;
extern crate ordered_float;
extern crate regex;
extern crate time;

#[macro_use]
extern crate lazy_static;

mod ast;
mod codegen;
mod compile_error;
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
            let context = inkwell::context::Context::create();
            let module = context.create_module("test");
            let target = codegen::TargetProperties::new(true);

            use codegen::controls::Control;
            use codegen::functions::Function;
            let codegen_start = time::precise_time_s();
            codegen::controls::AudioControl::build_funcs(&module, &target);
            codegen::functions::SinFunction::build_lifecycle_funcs(&module);
            codegen::functions::TanFunction::build_lifecycle_funcs(&module);

            codegen::block::build_construct_func(&module, &block, &target);
            codegen::block::build_update_func(&module, &block, &target);
            codegen::block::build_destruct_func(&module, &block, &target);
            println!("Codegen took {}s", time::precise_time_s() - codegen_start);

            println!("{:#?}", block);
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
    /*let groups = vec![
        ValueGroup::new(VarType::Num, None, None),
        ValueGroup::new(VarType::Num, None, None),
        ValueGroup::new(VarType::Num, None, None),
        ValueGroup::new(VarType::Num, None, None),
    ];
    let nodes = vec![
        Node::new(
            vec![ValueSocket::new(0, true, false, true)],
            Vec::new(),
            NodeData::Custom(BlockId::new_with_id("source1".to_string(), 0)),
        ),
        Node::new(
            vec![
                ValueSocket::new(0, false, true, false),
                ValueSocket::new(1, true, false, true),
            ],
            Vec::new(),
            NodeData::Custom(BlockId::new_with_id("reader1".to_string(), 1)),
        ),
        Node::new(
            vec![
                ValueSocket::new(1, false, true, false),
                ValueSocket::new(2, true, false, false),
            ],
            Vec::new(),
            NodeData::Custom(BlockId::new_with_id("reader2".to_string(), 2)),
        ),
        Node::new(
            vec![
                ValueSocket::new(2, false, true, true),
                ValueSocket::new(3, true, false, false),
            ],
            Vec::new(),
            NodeData::Custom(BlockId::new_with_id("reader3".to_string(), 3)),
        ),
        Node::new(
            vec![ValueSocket::new(3, false, true, true)],
            Vec::new(),
            NodeData::Custom(BlockId::new_with_id("reader4".to_string(), 4)),
        ),
    ];*/
    /*let mut allocator = MIRContext::new();
    let groups = vec![
        ValueGroup::new(VarType::Num, None, None),
        ValueGroup::new(VarType::Num, None, None),
        ValueGroup::new(VarType::Num, None, None),
    ];
    let nodes = vec![
        Node::new(
            vec![ValueSocket::new(1, true, false, true)],
            NodeData::Custom(BlockId::new("source".to_string(), &mut allocator)),
        ),
        Node::new(
            vec![
                ValueSocket::new(1, false, true, false),
                ValueSocket::new(2, false, true, false),
            ],
            NodeData::Custom(BlockId::new("middle".to_string(), &mut allocator)),
        ),
    ];
    let mut surface = Surface::new(
        SurfaceId::new("test".to_string(), &mut allocator),
        groups,
        nodes,
    );
    let group_start_time = time::precise_time_s();
    let new_surfaces = group_extracted(&mut surface, &mut allocator);
    println!(
        "Grouping took {}s",
        time::precise_time_s() - group_start_time
    );
    println!("Surface now: {:#?}", surface);
    println!("New surfaces: {:#?}", new_surfaces);*/

    loop {
        do_repl();
    }

    /*let context = inkwell::context::Context::create();
    let module = context.create_module("test");
    codegen::converters::build_convert_func(&module, &ast::FormType::None);
    codegen::converters::build_convert_func(&module, &ast::FormType::Amplitude);
    codegen::converters::build_convert_func(&module, &ast::FormType::Beats);
    codegen::converters::build_convert_func(&module, &ast::FormType::Control);
    codegen::converters::build_convert_func(&module, &ast::FormType::Db);
    codegen::converters::build_convert_func(&module, &ast::FormType::Frequency);
    codegen::converters::build_convert_func(&module, &ast::FormType::Note);
    codegen::converters::build_convert_func(&module, &ast::FormType::Oscillator);
    codegen::converters::build_convert_func(&module, &ast::FormType::Q);
    codegen::converters::build_convert_func(&module, &ast::FormType::Samples);
    codegen::converters::build_convert_func(&module, &ast::FormType::Seconds);

    use codegen::controls::Control;
    codegen::controls::build_funcs::<codegen::controls::AudioControl>(&module);
    codegen::controls::build_funcs::<codegen::controls::AudioExtractControl>(&module);
    codegen::controls::build_funcs::<codegen::controls::GraphControl>(&module);
    codegen::controls::build_funcs::<codegen::controls::MidiControl>(&module);
    codegen::controls::build_funcs::<codegen::controls::MidiExtractControl>(&module);
    codegen::controls::build_funcs::<codegen::controls::RollControl>(&module);
    codegen::controls::build_funcs::<codegen::controls::ScopeControl>(&module);

    if let Err(result_str) = module.verify() {
        println!("{}", result_str.to_string());
        return;
    }

    let pass_manager_builder = inkwell::passes::PassManagerBuilder::create();
    pass_manager_builder.set_optimization_level(inkwell::OptimizationLevel::Aggressive);
    pass_manager_builder.set_size_level(0);

    let module_pass = inkwell::passes::PassManager::create_for_module();
    pass_manager_builder.populate_module_pass_manager(&module_pass);

    let func_pass = inkwell::passes::PassManager::create_for_function(&module);
    pass_manager_builder.populate_function_pass_manager(&func_pass);

    func_pass.initialize();
    let func_iterator = ModuleFunctionIterator::new(&module);
    for func in func_iterator {
        func_pass.run_on_function(&func);
    }
    module_pass.run_on_module(&module);

    module.print_to_stderr();*/
}
