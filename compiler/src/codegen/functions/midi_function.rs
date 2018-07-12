use super::{Function, FunctionContext, VarArgs};
use ast::FormType;
use codegen::util;
use codegen::values::{MidiValue, NumValue, TupleValue};
use inkwell::context::Context;
use inkwell::types::StructType;
use inkwell::values::PointerValue;
use inkwell::IntPredicate;
use mir::block;

pub struct NoteFunction {}
impl Function for NoteFunction {
    fn function_type() -> block::Function {
        block::Function::Note
    }

    fn data_type(context: &Context) -> StructType {
        context.struct_type(
            &[
                &context.f32_type(), // last note
                &context.f32_type(), // last pitch
                &context.f32_type(), // last velocity
                &context.f32_type(), // last aftertouch
                &context.i8_type(),  // active count
            ],
            false,
        )
    }

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        _varargs: Option<VarArgs>,
        result: PointerValue,
    ) {
        let last_note_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 0, "lastnote.ptr")
        };
        let last_pitch_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 1, "lastpitch.ptr")
        };
        let last_velocity_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 2, "lastvelocity.ptr")
        };
        let last_aftertouch_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 3, "lastaftertouch.ptr")
        };
        let active_count_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 4, "activecount.ptr")
        };

        let in_midi = MidiValue::new(args[0]);
        let result_tuple = TupleValue::new(result);

        let iter_count = in_midi.get_count(func.ctx.b);
        let index_ptr = func.ctx
            .allocb
            .build_alloca(&func.ctx.context.i8_type(), "index.ptr");
        func.ctx
            .b
            .build_store(&index_ptr, &func.ctx.context.i8_type().const_int(0, false));

        let loop_check_block = func.ctx
            .context
            .append_basic_block(&func.ctx.func, "loop.check");
        let loop_run_block = func.ctx
            .context
            .append_basic_block(&func.ctx.func, "loop.run");
        let loop_end_block = func.ctx
            .context
            .append_basic_block(&func.ctx.func, "loop.end");
        func.ctx.b.build_unconditional_branch(&loop_check_block);

        func.ctx.b.position_at_end(&loop_check_block);
        let current_index = func.ctx.b.build_load(&index_ptr, "index").into_int_value();
        let branch_cond = func.ctx.b.build_int_compare(
            IntPredicate::ULT,
            current_index,
            iter_count,
            "branchcond",
        );
        func.ctx
            .b
            .build_conditional_branch(&branch_cond, &loop_run_block, &loop_end_block);

        func.ctx.b.position_at_end(&loop_run_block);
        let incr_index = func.ctx.b.build_int_add(
            current_index,
            func.ctx.context.i8_type().const_int(1, false),
            "index.increment",
        );
        func.ctx.b.build_store(&index_ptr, &incr_index);

        let event = in_midi.get_event(func.ctx.b, current_index);
        let event_name = event.get_name(func.ctx.b);
        let event_note = event.get_note(func.ctx.b);
        let event_param = event.get_param(func.ctx.b);

        // Note on event:
        //  - set last_note to event note
        //  - set last_velocity to normalized event velocity (stored in param)
        //  - increment active_count
        let note_on_block = func.ctx
            .context
            .append_basic_block(&func.ctx.func, "noteon");
        func.ctx.b.position_at_end(&note_on_block);
        let float_note = func.ctx.b.build_unsigned_int_to_float(
            event_note,
            func.ctx.context.f32_type(),
            "note.float",
        );
        func.ctx.b.build_store(&last_note_ptr, &float_note);
        let float_velocity = func.ctx.b.build_unsigned_int_to_float(
            event_param,
            func.ctx.context.f32_type(),
            "note.velocity",
        );
        let normalized_velocity = func.ctx.b.build_float_div(
            float_velocity,
            func.ctx.context.f32_type().const_float(255.),
            "velocity.normalized",
        );
        func.ctx
            .b
            .build_store(&last_velocity_ptr, &normalized_velocity);
        let incremented_active = func.ctx.b.build_int_add(
            func.ctx
                .b
                .build_load(&active_count_ptr, "activecount")
                .into_int_value(),
            func.ctx.context.i8_type().const_int(1, false),
            "active.incremented",
        );
        func.ctx
            .b
            .build_store(&active_count_ptr, &incremented_active);
        func.ctx.b.build_unconditional_branch(&loop_check_block);

        // Note off event:
        //  - decrement active_count
        // Todo: handle underflow?
        let note_off_block = func.ctx
            .context
            .append_basic_block(&func.ctx.func, "noteoff");
        func.ctx.b.position_at_end(&note_off_block);
        let decremented_active = func.ctx.b.build_int_sub(
            func.ctx
                .b
                .build_load(&active_count_ptr, "activecount")
                .into_int_value(),
            func.ctx.context.i8_type().const_int(1, false),
            "active.decremented",
        );
        func.ctx
            .b
            .build_store(&active_count_ptr, &decremented_active);
        func.ctx.b.build_unconditional_branch(&loop_check_block);

        // Pitch wheel event:
        //  - set last_pitch_wheel to normalized pitch wheel (stored in param)
        let pitch_wheel_block = func.ctx
            .context
            .append_basic_block(&func.ctx.func, "pitchwheel");
        func.ctx.b.position_at_end(&pitch_wheel_block);
        let pitch_float = func.ctx.b.build_unsigned_int_to_float(
            event_param,
            func.ctx.context.f32_type(),
            "pitch",
        );
        // remap the pitch from {0,255} to {-6,6}
        let normalized_pitch = func.ctx.b.build_float_mul(
            func.ctx.b.build_float_sub(
                func.ctx.b.build_float_div(
                    pitch_float,
                    func.ctx.context.f32_type().const_float(127.5),
                    "",
                ),
                func.ctx.context.f32_type().const_float(1.),
                "",
            ),
            func.ctx.context.f32_type().const_float(6.),
            "pitch.normalized",
        );
        func.ctx.b.build_store(&last_pitch_ptr, &normalized_pitch);
        func.ctx.b.build_unconditional_branch(&loop_check_block);

        // Aftertouch event:
        //  - set last_aftertouch to normalized aftertouch
        let aftertouch_block = func.ctx
            .context
            .append_basic_block(&func.ctx.func, "aftertouch");
        func.ctx.b.position_at_end(&aftertouch_block);
        let aftertouch_float = func.ctx.b.build_unsigned_int_to_float(
            event_param,
            func.ctx.context.f32_type(),
            "aftertouch",
        );
        let normalized_aftertouch = func.ctx.b.build_float_div(
            aftertouch_float,
            func.ctx.context.f32_type().const_float(255.),
            "aftertouch.normalized",
        );
        func.ctx
            .b
            .build_store(&last_aftertouch_ptr, &normalized_aftertouch);
        func.ctx.b.build_unconditional_branch(&loop_check_block);

        // build the switch that handles each event
        func.ctx.b.position_at_end(&loop_run_block);
        func.ctx.b.build_switch(
            &event_name,
            &loop_check_block,
            &[
                (
                    &func.ctx.context.i8_type().const_int(0, false),
                    &note_on_block,
                ), // 0 = note on
                (
                    &func.ctx.context.i8_type().const_int(1, false),
                    &note_off_block,
                ), // 1 = note off
                (
                    &func.ctx.context.i8_type().const_int(2, false),
                    &aftertouch_block,
                ), // 2 = polyphonic aftertouch
                (
                    &func.ctx.context.i8_type().const_int(3, false),
                    &aftertouch_block,
                ), // 3 = channel aftertouch
                (
                    &func.ctx.context.i8_type().const_int(4, false),
                    &pitch_wheel_block,
                ), // 4 = pitch wheel
            ],
        );

        func.ctx.b.position_at_end(&loop_end_block);

        let is_active = func.ctx.b.build_unsigned_int_to_float(
            func.ctx.b.build_int_compare(
                IntPredicate::UGT,
                func.ctx
                    .b
                    .build_load(&active_count_ptr, "activecount")
                    .into_int_value(),
                func.ctx.context.i8_type().const_int(0, false),
                "active",
            ),
            func.ctx.context.f32_type(),
            "activefloat",
        );
        let note_num = func.ctx.b.build_float_add(
            func.ctx
                .b
                .build_load(&last_note_ptr, "note")
                .into_float_value(),
            func.ctx
                .b
                .build_load(&last_pitch_ptr, "pitch")
                .into_float_value(),
            "notenum",
        );
        let velocity_num = func.ctx
            .b
            .build_load(&last_velocity_ptr, "velocity")
            .into_float_value();
        let aftertouch_num = func.ctx
            .b
            .build_load(&last_aftertouch_ptr, "aftertouch")
            .into_float_value();

        let result_gate_num = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 0));
        let active_vector = util::splat_vector(func.ctx.b, is_active, "active.vector");
        result_gate_num.set_vec(func.ctx.b, &active_vector);
        result_gate_num.set_form(
            func.ctx.b,
            &func.ctx
                .context
                .i8_type()
                .const_int(FormType::None as u64, false),
        );

        let result_note_num = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 1));
        let note_vector = util::splat_vector(func.ctx.b, note_num, "note.vector");
        result_note_num.set_vec(func.ctx.b, &note_vector);
        result_note_num.set_form(
            func.ctx.b,
            &func.ctx
                .context
                .i8_type()
                .const_int(FormType::Note as u64, false),
        );

        let result_velocity_num = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 2));
        let velocity_vector = util::splat_vector(func.ctx.b, velocity_num, "velocity.vector");
        result_velocity_num.set_vec(func.ctx.b, &velocity_vector);
        result_velocity_num.set_form(
            func.ctx.b,
            &func.ctx
                .context
                .i8_type()
                .const_int(FormType::None as u64, false),
        );

        let result_aftertouch_num = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 3));
        let aftertouch_vector = util::splat_vector(func.ctx.b, aftertouch_num, "aftertouch.vector");
        result_aftertouch_num.set_vec(func.ctx.b, &aftertouch_vector);
        result_aftertouch_num.set_form(
            func.ctx.b,
            &func.ctx
                .context
                .i8_type()
                .const_int(FormType::None as u64, false),
        );
    }
}
