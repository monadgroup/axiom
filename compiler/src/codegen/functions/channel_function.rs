use super::{Function, FunctionContext, VarArgs};
use codegen::intrinsics;
use codegen::values::{MidiValue, NumValue};
use inkwell::values::PointerValue;
use inkwell::IntPredicate;
use mir::block;

pub struct ChannelFunction {}
impl Function for ChannelFunction {
    fn function_type() -> block::Function {
        block::Function::Channel
    }

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        _varargs: Option<VarArgs>,
        result: PointerValue,
    ) {
        let min_intrinsic = intrinsics::minnum_f32(func.ctx.module);
        let max_intrinsic = intrinsics::maxnum_f32(func.ctx.module);

        let input_midi = MidiValue::new(args[0]);
        let channel_num = NumValue::new(args[1]);
        let result_midi = MidiValue::new(result);

        result_midi.set_count(func.ctx.b, &func.ctx.context.i8_type().const_int(0, false));

        let channel_vec = channel_num.get_vec(func.ctx.b);

        // clamp the channel to {0,16}
        let channel_float = func
            .ctx
            .b
            .build_call(
                &min_intrinsic,
                &[
                    &func
                        .ctx
                        .b
                        .build_call(
                            &max_intrinsic,
                            &[
                                &func.ctx.b.build_extract_element(
                                    &channel_vec,
                                    &func.ctx.context.i32_type().const_int(0, false),
                                    "",
                                ),
                                &func.ctx.context.f32_type().const_float(0.),
                            ],
                            "",
                            false,
                        ).left()
                        .unwrap()
                        .into_float_value(),
                    &func.ctx.context.f32_type().const_float(16.),
                ],
                "",
                false,
            ).left()
            .unwrap()
            .into_float_value();
        let channel_int =
            func.ctx
                .b
                .build_float_to_unsigned_int(channel_float, func.ctx.context.i8_type(), "");

        // loop over each event and determine if it's for this channel
        let event_count = input_midi.get_count(func.ctx.b);
        let index_ptr = func
            .ctx
            .allocb
            .build_alloca(&func.ctx.context.i8_type(), "index.ptr");
        func.ctx
            .b
            .build_store(&index_ptr, &func.ctx.context.i8_type().const_int(0, false));

        let loop_check_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "loop.check");
        let loop_run_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "loop.run");
        let loop_match_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "loop.match");
        let loop_end_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "loop.end");

        func.ctx.b.build_unconditional_branch(&loop_check_block);

        func.ctx.b.position_at_end(&loop_check_block);
        let current_index = func.ctx.b.build_load(&index_ptr, "index").into_int_value();
        let index_cond = func.ctx.b.build_int_compare(
            IntPredicate::ULT,
            current_index,
            event_count,
            "indexcond",
        );
        func.ctx
            .b
            .build_conditional_branch(&index_cond, &loop_run_block, &loop_end_block);

        func.ctx.b.position_at_end(&loop_run_block);
        let next_index = func.ctx.b.build_int_add(
            current_index,
            func.ctx.context.i8_type().const_int(1, false),
            "nextindex",
        );
        func.ctx.b.build_store(&index_ptr, &next_index);

        let current_event = input_midi.get_event(func.ctx.b, current_index);
        let event_channel = current_event.get_channel(func.ctx.b);
        let channel_cond = func.ctx.b.build_int_compare(
            IntPredicate::EQ,
            event_channel,
            channel_int,
            "channelcond",
        );
        func.ctx
            .b
            .build_conditional_branch(&channel_cond, &loop_match_block, &loop_check_block);

        func.ctx.b.position_at_end(&loop_match_block);
        result_midi.push_event(func.ctx.b, func.ctx.module, &current_event);
        func.ctx.b.build_unconditional_branch(&loop_check_block);

        func.ctx.b.position_at_end(&loop_end_block);
    }
}
