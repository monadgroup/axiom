use super::{Function, FunctionContext, VarArgs};
use crate::codegen::util;
use crate::codegen::values::{ArrayValue, MidiValue, NumValue, ARRAY_CAPACITY};
use crate::mir::block;
use inkwell::context::Context;
use inkwell::types::StructType;
use inkwell::values::PointerValue;
use inkwell::{FloatPredicate, IntPredicate};

pub struct VoicesFunction {}
impl Function for VoicesFunction {
    fn function_type() -> block::Function {
        block::Function::Voices
    }

    fn data_type(context: &Context) -> StructType {
        context.struct_type(
            &[
                &context.i8_type().array_type(u32::from(ARRAY_CAPACITY)), // assigned notes
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
        let current_notes_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 0, "currentnotes.ptr")
        };

        let input_midi = MidiValue::new(args[0]);
        let last_active_array = ArrayValue::new(args[1]);
        let result_array = ArrayValue::new(result);
        result_array.set_bitmap(func.ctx.b, func.ctx.context.i32_type().const_int(0, false));

        let init_index_ptr = func
            .ctx
            .allocb
            .build_alloca(&func.ctx.context.i8_type(), "initindex.ptr");
        let event_index_ptr = func
            .ctx
            .allocb
            .build_alloca(&func.ctx.context.i8_type(), "eventindex.ptr");
        let inner_index_ptr = func
            .ctx
            .allocb
            .build_alloca(&func.ctx.context.i8_type(), "innerindex.ptr");

        func.ctx.b.build_store(
            &init_index_ptr,
            &func.ctx.context.i8_type().const_int(0, false),
        );
        func.ctx.b.build_store(
            &event_index_ptr,
            &func.ctx.context.i8_type().const_int(0, false),
        );

        let event_count = input_midi.get_count(func.ctx.b);

        let init_loop_check_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "initloop.check");
        let init_loop_run_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "initloop.run");
        let init_active_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "initactive.true");

        let event_loop_check_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "eventloop.check");
        let event_loop_run_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "eventloop.run");
        let event_loop_finish_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "eventloop.finish");

        let note_on_loop_check_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "noteonloop.check");
        let note_on_loop_run_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "noteonloop.run");
        let note_on_loop_assign_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "noteonloop.assign");

        let note_else_loop_check_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "noteelseloop.check");
        let note_else_loop_run_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "noteelseloop.run");
        let note_else_loop_active_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "noteelseloop.active");
        let note_else_loop_assign_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "noteelseloop.assign");

        func.ctx
            .b
            .build_unconditional_branch(&init_loop_check_block);

        // Build the init loop, which iterates over each voice, clearing MIDI counts in the result
        // and making outputs active if the inputs are.
        {
            func.ctx.b.position_at_end(&init_loop_check_block);
            let current_init_index = func.ctx.b.build_load(&init_index_ptr, "").into_int_value();
            let init_cond = func.ctx.b.build_int_compare(
                IntPredicate::ULT,
                current_init_index,
                func.ctx
                    .context
                    .i8_type()
                    .const_int(u64::from(ARRAY_CAPACITY), false),
                "",
            );
            func.ctx.b.build_conditional_branch(
                &init_cond,
                &init_loop_run_block,
                &event_loop_check_block,
            );

            func.ctx.b.position_at_end(&init_loop_run_block);
            let incremented_init_index = func.ctx.b.build_int_add(
                current_init_index,
                func.ctx.context.i8_type().const_int(1, false),
                "nextinitindex",
            );
            func.ctx
                .b
                .build_store(&init_index_ptr, &incremented_init_index);

            let init_last_active =
                NumValue::new(last_active_array.get_item_ptr(func.ctx.b, current_init_index));
            let init_midi =
                MidiValue::new(result_array.get_item_ptr(func.ctx.b, current_init_index));
            init_midi.set_count(func.ctx.b, func.ctx.context.i8_type().const_int(0, false));

            let init_last_active_vec = init_last_active.get_vec(func.ctx.b);
            let active_cond = func.ctx.b.build_float_compare(
                FloatPredicate::ONE,
                func.ctx
                    .b
                    .build_extract_element(
                        &init_last_active_vec,
                        &func.ctx.context.i32_type().const_int(0, false),
                        "initactiveval",
                    )
                    .into_float_value(),
                func.ctx.context.f64_type().const_float(0.),
                "activecond",
            );
            func.ctx.b.build_conditional_branch(
                &active_cond,
                &init_active_block,
                &init_loop_check_block,
            );

            func.ctx.b.position_at_end(&init_active_block);

            // set the bitmap in the result bitmap if the voice is active
            let current_result_bitmap = result_array.get_bitmap(func.ctx.b);
            let new_bitmap = util::set_bit(func.ctx.b, current_result_bitmap, current_init_index);
            result_array.set_bitmap(func.ctx.b, new_bitmap);
            func.ctx
                .b
                .build_unconditional_branch(&init_loop_check_block);
        }

        // Loop over each event in the input and distribute to the output voices:
        //  - If the input event is a note on event, find an available voice slot (i.e one that isn't active) and claim it
        //  - If the input event is a note-specific event, find the voice set to that note and pass through the event
        //  - If the input event is a global event, pass it into all voices
        {
            func.ctx.b.position_at_end(&event_loop_check_block);
            let current_event_index = func.ctx.b.build_load(&event_index_ptr, "").into_int_value();
            let event_cond = func.ctx.b.build_int_compare(
                IntPredicate::ULT,
                current_event_index,
                event_count,
                "eventcond",
            );
            func.ctx.b.build_conditional_branch(
                &event_cond,
                &event_loop_run_block,
                &event_loop_finish_block,
            );

            func.ctx.b.position_at_end(&event_loop_run_block);
            let incremented_event_index = func.ctx.b.build_int_nuw_add(
                current_event_index,
                func.ctx.context.i8_type().const_int(1, false),
                "nexteventindex",
            );
            func.ctx
                .b
                .build_store(&event_index_ptr, &incremented_event_index);

            let current_event = input_midi.get_event(func.ctx.b, current_event_index);
            let current_event_name = current_event.get_name(func.ctx.b);
            let is_on_cond = func.ctx.b.build_int_compare(
                IntPredicate::EQ,
                current_event_name,
                func.ctx.context.i8_type().const_int(0, false),
                "oncond",
            );
            func.ctx.b.build_store(
                &inner_index_ptr,
                &func.ctx.context.i8_type().const_int(0, false),
            );
            func.ctx.b.build_conditional_branch(
                &is_on_cond,
                &note_on_loop_check_block,
                &note_else_loop_check_block,
            );

            {
                func.ctx.b.position_at_end(&note_on_loop_check_block);
                let current_active_index =
                    func.ctx.b.build_load(&inner_index_ptr, "").into_int_value();
                let active_cond = func.ctx.b.build_int_compare(
                    IntPredicate::ULT,
                    current_active_index,
                    func.ctx
                        .context
                        .i8_type()
                        .const_int(u64::from(ARRAY_CAPACITY), false),
                    "activecond",
                );
                func.ctx.b.build_conditional_branch(
                    &active_cond,
                    &note_on_loop_run_block,
                    &event_loop_check_block,
                );

                func.ctx.b.position_at_end(&note_on_loop_run_block);
                let incremented_active_index = func.ctx.b.build_int_nuw_add(
                    current_active_index,
                    func.ctx.context.i8_type().const_int(1, false),
                    "nextactiveindex",
                );
                func.ctx
                    .b
                    .build_store(&inner_index_ptr, &incremented_active_index);

                let current_result_bitmap = result_array.get_bitmap(func.ctx.b);
                let is_output_active =
                    util::get_bit(func.ctx.b, current_result_bitmap, current_active_index);
                func.ctx.b.build_conditional_branch(
                    &is_output_active,
                    &note_on_loop_check_block,
                    &note_on_loop_assign_block,
                );

                func.ctx.b.position_at_end(&note_on_loop_assign_block);
                // claim the note - set the voice in the array, set the bitmap on the output,
                // and push the event to the output voice
                let assign_note_ptr = unsafe {
                    func.ctx.b.build_in_bounds_gep(
                        &current_notes_ptr,
                        &[
                            func.ctx.context.i64_type().const_int(0, false),
                            current_active_index,
                        ],
                        "noteassign.ptr",
                    )
                };
                let current_event_note = current_event.get_note(func.ctx.b);
                func.ctx
                    .b
                    .build_store(&assign_note_ptr, &current_event_note);
                let new_bitmap =
                    util::set_bit(func.ctx.b, current_result_bitmap, current_active_index);
                result_array.set_bitmap(func.ctx.b, new_bitmap);

                let output_midi =
                    MidiValue::new(result_array.get_item_ptr(func.ctx.b, current_active_index));
                output_midi.push_event(func.ctx.b, func.ctx.module, &current_event);
                func.ctx
                    .b
                    .build_unconditional_branch(&event_loop_check_block);
            }

            {
                func.ctx.b.position_at_end(&note_else_loop_check_block);
                let current_note_index =
                    func.ctx.b.build_load(&inner_index_ptr, "").into_int_value();
                let note_cond = func.ctx.b.build_int_compare(
                    IntPredicate::ULT,
                    current_note_index,
                    func.ctx
                        .context
                        .i8_type()
                        .const_int(u64::from(ARRAY_CAPACITY), false),
                    "notecond",
                );
                func.ctx.b.build_conditional_branch(
                    &note_cond,
                    &note_else_loop_run_block,
                    &event_loop_check_block,
                );

                func.ctx.b.position_at_end(&note_else_loop_run_block);
                let incremented_note_index = func.ctx.b.build_int_nuw_add(
                    current_note_index,
                    func.ctx.context.i8_type().const_int(1, false),
                    "noteindex",
                );
                func.ctx
                    .b
                    .build_store(&inner_index_ptr, &incremented_note_index);

                // determine if the voice is active and matches the note in the event, or the event doesn't have an event
                let current_result_bitmap = result_array.get_bitmap(func.ctx.b);
                let is_output_active =
                    util::get_bit(func.ctx.b, current_result_bitmap, current_note_index);
                func.ctx.b.build_conditional_branch(
                    &is_output_active,
                    &note_else_loop_active_block,
                    &note_else_loop_check_block,
                );

                func.ctx.b.position_at_end(&note_else_loop_active_block);
                let current_note_ptr = unsafe {
                    func.ctx.b.build_in_bounds_gep(
                        &current_notes_ptr,
                        &[
                            func.ctx.context.i64_type().const_int(0, false),
                            current_note_index,
                        ],
                        "noteindex.ptr",
                    )
                };
                let current_note = func
                    .ctx
                    .b
                    .build_load(&current_note_ptr, "noteindex")
                    .into_int_value();
                let current_event_note = current_event.get_note(func.ctx.b);
                let notes_equal_cond = func.ctx.b.build_int_compare(
                    IntPredicate::EQ,
                    current_note,
                    current_event_note,
                    "notecond",
                );

                let current_event_name = current_event.get_name(func.ctx.b);
                let channel_aftertouch_cond = func.ctx.b.build_int_compare(
                    IntPredicate::EQ,
                    current_event_name,
                    func.ctx.context.i8_type().const_int(3, false), // 3 = channel aftertouch
                    "channelaftertouchcond",
                );
                let channel_pitchwheel_cond = func.ctx.b.build_int_compare(
                    IntPredicate::EQ,
                    current_event_name,
                    func.ctx.context.i8_type().const_int(4, false), // 4 = pitchwheel
                    "channelpitchwheelcond",
                );
                let is_channel_event_cond = func.ctx.b.build_or(
                    channel_aftertouch_cond,
                    channel_pitchwheel_cond,
                    "channeleventcond",
                );
                let queue_cond =
                    func.ctx
                        .b
                        .build_or(notes_equal_cond, is_channel_event_cond, "queuecond");
                func.ctx.b.build_conditional_branch(
                    &queue_cond,
                    &note_else_loop_assign_block,
                    &note_else_loop_check_block,
                );

                func.ctx.b.position_at_end(&note_else_loop_assign_block);
                let target_midi =
                    MidiValue::new(result_array.get_item_ptr(func.ctx.b, current_note_index));
                target_midi.push_event(func.ctx.b, func.ctx.module, &current_event);
                func.ctx
                    .b
                    .build_unconditional_branch(&note_else_loop_check_block);
            }
        }

        func.ctx.b.position_at_end(&event_loop_finish_block);
    }
}
