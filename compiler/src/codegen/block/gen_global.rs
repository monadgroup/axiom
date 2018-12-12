use super::BlockContext;
use crate::ast::FormType;
use crate::codegen::globals;
use crate::codegen::values::NumValue;
use crate::mir::block::Global;
use inkwell::values::PointerValue;

pub fn gen_global_statement(global: &Global, node: &mut BlockContext) -> PointerValue {
    let num_val = NumValue::new_undef(node.ctx.context, node.ctx.allocb);
    let vec_ptr = match global {
        Global::SampleRate => globals::get_sample_rate(node.ctx.module).as_pointer_value(),
        Global::BPM => globals::get_bpm(node.ctx.module).as_pointer_value(),
    };
    let vec_val = node
        .ctx
        .b
        .build_load(&vec_ptr, "global.vec")
        .into_vector_value();
    num_val.set_form(
        node.ctx.b,
        &node
            .ctx
            .context
            .i8_type()
            .const_int(FormType::None as u64, false),
    );
    num_val.set_vec(node.ctx.b, &vec_val);
    num_val.val
}
