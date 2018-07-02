extern crate divrem;
extern crate inkwell;
extern crate ordered_float;
extern crate regex;
extern crate time;

#[macro_use]
extern crate lazy_static;

mod compile_error;

pub mod ast;
pub mod codegen;
pub mod frontend;
pub mod mir;
pub mod parser;
pub mod pass;
pub mod util;

pub use compile_error::{CompileError, CompileResult};

// C api
pub use frontend::c_api::{
    maxim_allocate_id, maxim_build_block, maxim_build_custom_node, maxim_build_group_node,
    maxim_build_surface, maxim_build_value_group, maxim_build_value_socket, maxim_commit,
    maxim_compile_block, maxim_constant_clone, maxim_constant_num, maxim_constant_tuple,
    maxim_create_runtime, maxim_create_transaction, maxim_destroy_error, maxim_destroy_runtime,
    maxim_initialize, maxim_valuegroupsource_default, maxim_valuegroupsource_none,
    maxim_valuegroupsource_socket, maxim_vartype_array, maxim_vartype_clone, maxim_vartype_midi,
    maxim_vartype_num, maxim_vartype_tuple,
};
