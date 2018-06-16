extern crate ordered_float;
extern crate regex;
extern crate time;

#[macro_use]
extern crate lazy_static;

mod ast;
mod compile_error;
mod mir;
mod parser;
mod pass;
mod util;

pub use self::ast::*;
pub use self::compile_error::{CompileError, CompileResult};
pub use self::mir::*;
pub use self::parser::*;
pub use self::pass::*;
pub use self::util::*;

/*fn run_code(code: &str) {
    let mut stream = get_token_stream(code);

    let parse_start_time = time::precise_time_s();
    let ast = Parser::parse(&mut stream);
    println!("Parse took {}s", time::precise_time_s() - parse_start_time);

    let mir = ast.and_then(|ast| {
        let lower_start = time::precise_time_s();
        let block = lower_ast(mir::BlockId::new_with_id("test".to_owned(), 0), &ast);
        println!("Lower took {}s", time::precise_time_s() - lower_start);
        block
    });

    match mir {
        Ok(mut block) => {
            remove_dead_code(&mut block);
            println!("{:#?}", block)
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
    let mut input = "".to_owned();
    for line in stdin.lock().lines() {
        let unwrapped = line.unwrap();
        input.push_str(&unwrapped);
        input.push_str("\n");

        if unwrapped.is_empty() {
            run_code(&input);
            return;
        }
    }
}*/

fn main() {
    // build a basic MIR
    let groups = vec![ControlGroup::new(VarType::Num, true, true, None)];
    let nodes = vec![
        Node::new(
            vec![Control::new(ControlType::NumExtract, 0, true, false)],
            NodeData::Custom(BlockId::new_with_id("source1".to_string(), 0)),
        ),
        Node::new(
            vec![Control::new(ControlType::NumExtract, 0, false, true)],
            NodeData::Custom(BlockId::new_with_id("source2".to_string(), 1)),
        ),
        Node::new(
            vec![Control::new(ControlType::Audio, 0, false, true)],
            NodeData::Custom(BlockId::new_with_id("consumer1".to_string(), 2)),
        ),
    ];
    let mut surface = Surface::new(SurfaceId::new_with_id("test".to_string(), 0), groups, nodes);
    let group_start_time = time::precise_time_s();
    group_extracted(&mut surface);
    println!(
        "Grouping took {}s",
        time::precise_time_s() - group_start_time
    );
}
