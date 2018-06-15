mod lower_ast;
mod remove_dead_code;

pub use self::lower_ast::lower_ast;
pub use self::remove_dead_code::remove_dead_code;
