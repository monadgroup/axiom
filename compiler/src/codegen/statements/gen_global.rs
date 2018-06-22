use codegen::values::NumValue;
use codegen::{globals, NodeContext};
use inkwell::values::PointerValue;
use mir::block::Global;

pub fn gen_global_statement(global: &Global, node: &mut NodeContext) -> PointerValue {
    let num_val = NumValue::new_undef(node.context, node.alloca_builder);
    let vec_ptr = match global {
        Global::SampleRate => globals::get_sample_rate(node.module),
        Global::BPM => globals::get_bpm(node.module),
    };
    let vec_val = node.builder
        .build_load(&vec_ptr, "global.vec")
        .into_vector_value();
    num_val.set_vec(node.builder, &vec_val);
    num_val.val
}
