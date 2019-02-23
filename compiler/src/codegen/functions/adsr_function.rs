use super::{Function, FunctionContext, VarArgs};
use crate::ast::FormType;
use crate::codegen::values::{NumValue, TupleValue};
use crate::codegen::{
    build_context_function, globals, math, util, BuilderContext, TargetProperties,
};
use crate::mir::block;
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::StructType;
use inkwell::values::{FunctionValue, PointerValue};
use inkwell::{AddressSpace, FloatPredicate, IntPredicate};

#[repr(u8)]
enum EnvelopeState {
    NotActive,
    Attack,
    Decay,
    Sustain,
    Release,
}

pub struct AdsrFunction;

fn get_channel_update_data_type(context: &Context) -> StructType {
    context.struct_type(
        &[
            &context.bool_type(), // last trigger
            &context.f64_type(),  // position
            &context.f64_type(),  // release value
            &context.f64_type(),  // last value
            &context.i8_type(),   // state
        ],
        false,
    )
}

fn get_channel_update_func(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.util.adsr.channelUpdate", true, &|| {
        let context = &module.get_context();
        let result_type = context.struct_type(&[&context.bool_type(), &context.f64_type()], false);
        (
            Linkage::PrivateLinkage,
            result_type.fn_type(
                &[
                    &get_channel_update_data_type(context).ptr_type(AddressSpace::Generic),
                    &context.bool_type(), // current trigger
                    &context.f64_type(),  // attack
                    &context.f64_type(),  // decay
                    &context.f64_type(),  // sustain
                    &context.f64_type(),  // release
                ],
                false,
            ),
        )
    })
}

fn build_channel_update_func(module: &Module, target: &TargetProperties) {
    let func = get_channel_update_func(module);
    build_context_function(module, func, target, &|ctx: BuilderContext| {
        let pow_intrinsic = math::pow_v2f64(module);

        let state_not_active_const = ctx
            .context
            .i8_type()
            .const_int(EnvelopeState::NotActive as u64, false);
        let state_attack_const = ctx
            .context
            .i8_type()
            .const_int(EnvelopeState::Attack as u64, false);
        let state_decay_const = ctx
            .context
            .i8_type()
            .const_int(EnvelopeState::Decay as u64, false);
        let state_sustain_const = ctx
            .context
            .i8_type()
            .const_int(EnvelopeState::Sustain as u64, false);
        let state_release_const = ctx
            .context
            .i8_type()
            .const_int(EnvelopeState::Release as u64, false);

        let data_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();
        let trigger_bool = ctx.func.get_nth_param(1).unwrap().into_int_value();
        let attack_float = ctx.func.get_nth_param(2).unwrap().into_float_value();
        let decay_float = ctx.func.get_nth_param(3).unwrap().into_float_value();
        let sustain_float = ctx.func.get_nth_param(4).unwrap().into_float_value();
        let release_float = ctx.func.get_nth_param(5).unwrap().into_float_value();

        let trig_changed_true_block = ctx
            .context
            .append_basic_block(&ctx.func, "trigchanged.true");
        let trig_true_block = ctx.context.append_basic_block(&ctx.func, "trig.true");
        let trig_false_block = ctx.context.append_basic_block(&ctx.func, "trig.false");
        let trig_changed_continue_block = ctx
            .context
            .append_basic_block(&ctx.func, "trigchanged.continue");
        let active_true_block = ctx.context.append_basic_block(&ctx.func, "active.true");
        let pos_state_attack_block = ctx
            .context
            .append_basic_block(&ctx.func, "pos.state.attack");
        let pos_state_attack_finished_block = ctx
            .context
            .append_basic_block(&ctx.func, "pos.state.attack.finished");
        let pos_state_decay_block = ctx.context.append_basic_block(&ctx.func, "pos.state.decay");
        let pos_state_decay_finished_block = ctx
            .context
            .append_basic_block(&ctx.func, "pos.state.decay.finished");
        let pos_state_release_block = ctx
            .context
            .append_basic_block(&ctx.func, "pos.state.release");
        let pos_state_release_finished_block = ctx
            .context
            .append_basic_block(&ctx.func, "pos.state.release.finished");
        let active_continue_block = ctx.context.append_basic_block(&ctx.func, "active.continue");
        let val_state_attack_block = ctx
            .context
            .append_basic_block(&ctx.func, "val.state.attack");
        let val_state_decay_block = ctx.context.append_basic_block(&ctx.func, "val.state.decay");
        let val_state_sustain_block = ctx
            .context
            .append_basic_block(&ctx.func, "val.state.sustain");
        let val_state_release_block = ctx
            .context
            .append_basic_block(&ctx.func, "val.state.release");
        let val_state_default_block = ctx
            .context
            .append_basic_block(&ctx.func, "val.state.default");
        let val_state_continue_block = ctx
            .context
            .append_basic_block(&ctx.func, "val.state.continue");

        let last_trigger_ptr = unsafe { ctx.b.build_struct_gep(&data_ptr, 0, "lasttrig.ptr") };
        let position_ptr = unsafe { ctx.b.build_struct_gep(&data_ptr, 1, "position.ptr") };
        let release_val_ptr = unsafe { ctx.b.build_struct_gep(&data_ptr, 2, "releaseval.ptr") };
        let last_val_ptr = unsafe { ctx.b.build_struct_gep(&data_ptr, 3, "lastval.ptr") };
        let state_ptr = unsafe { ctx.b.build_struct_gep(&data_ptr, 4, "state.ptr") };

        let last_trigger = ctx
            .b
            .build_load(&last_trigger_ptr, "lasttrig")
            .into_int_value();
        let trig_changed =
            ctx.b
                .build_int_compare(IntPredicate::NE, trigger_bool, last_trigger, "trigchanged");
        ctx.b.build_conditional_branch(
            &trig_changed,
            &trig_changed_true_block,
            &trig_changed_continue_block,
        );

        ctx.b.position_at_end(&trig_changed_true_block);
        ctx.b
            .build_conditional_branch(&trigger_bool, &trig_true_block, &trig_false_block);

        ctx.b.position_at_end(&trig_true_block);
        ctx.b.build_store(&state_ptr, &state_attack_const);
        ctx.b
            .build_store(&position_ptr, &ctx.context.f64_type().const_float(0.));
        ctx.b
            .build_unconditional_branch(&trig_changed_continue_block);

        ctx.b.position_at_end(&trig_false_block);
        let last_val = ctx
            .b
            .build_load(&last_val_ptr, "lastval")
            .into_float_value();
        ctx.b.build_store(&release_val_ptr, &last_val);
        let new_state = ctx
            .b
            .build_select(
                ctx.b.build_float_compare(
                    FloatPredicate::OGT,
                    release_float,
                    ctx.context.f64_type().const_float(0.),
                    "",
                ),
                state_release_const,
                state_not_active_const,
                "",
            )
            .into_int_value();
        ctx.b.build_store(&state_ptr, &new_state);
        ctx.b
            .build_store(&position_ptr, &ctx.context.f64_type().const_float(0.));
        ctx.b
            .build_unconditional_branch(&trig_changed_continue_block);

        ctx.b.position_at_end(&trig_changed_continue_block);
        ctx.b.build_store(&last_trigger_ptr, &trigger_bool);

        let current_state = ctx.b.build_load(&state_ptr, "state").into_int_value();
        let is_active = ctx.b.build_int_compare(
            IntPredicate::NE,
            current_state,
            state_not_active_const,
            "isactive",
        );
        ctx.b
            .build_conditional_branch(&is_active, &active_true_block, &active_continue_block);

        ctx.b.position_at_end(&active_true_block);
        let pos_delta = ctx.b.build_float_div(
            ctx.context.f64_type().const_float(1.),
            ctx.b
                .build_extract_element(
                    &ctx.b
                        .build_load(
                            &globals::get_sample_rate(ctx.module).as_pointer_value(),
                            "samplerate",
                        )
                        .into_vector_value(),
                    &ctx.context.i64_type().const_int(0, false),
                    "samplerate.left",
                )
                .into_float_value(),
            "posdelta",
        );

        ctx.b.build_switch(
            &current_state,
            &active_continue_block,
            &[
                (&state_attack_const, &pos_state_attack_block),
                (&state_decay_const, &pos_state_decay_block),
                (&state_release_const, &pos_state_release_block),
            ],
        );

        ctx.b.position_at_end(&pos_state_attack_block);
        let new_pos = ctx.b.build_float_add(
            ctx.b.build_load(&position_ptr, "pos").into_float_value(),
            pos_delta,
            "newpos",
        );
        ctx.b.build_store(&position_ptr, &new_pos);
        let is_attack_finished = ctx.b.build_float_compare(
            FloatPredicate::OGE,
            new_pos,
            attack_float,
            "attack.finished",
        );
        ctx.b.build_conditional_branch(
            &is_attack_finished,
            &pos_state_attack_finished_block,
            &active_continue_block,
        );

        ctx.b.position_at_end(&pos_state_attack_finished_block);
        ctx.b
            .build_store(&position_ptr, &ctx.context.f64_type().const_float(0.));
        ctx.b.build_store(
            &state_ptr,
            &ctx.b.build_select(
                ctx.b.build_float_compare(
                    FloatPredicate::OGT,
                    decay_float,
                    ctx.context.f64_type().const_float(0.),
                    "",
                ),
                state_decay_const,
                state_sustain_const,
                "",
            ),
        );
        ctx.b.build_unconditional_branch(&active_continue_block);

        ctx.b.position_at_end(&pos_state_decay_block);
        let new_pos = ctx.b.build_float_add(
            ctx.b.build_load(&position_ptr, "pos").into_float_value(),
            pos_delta,
            "newpos",
        );
        ctx.b.build_store(&position_ptr, &new_pos);
        let is_decay_finished =
            ctx.b
                .build_float_compare(FloatPredicate::OGE, new_pos, decay_float, "decay.finished");
        ctx.b.build_conditional_branch(
            &is_decay_finished,
            &pos_state_decay_finished_block,
            &active_continue_block,
        );

        ctx.b.position_at_end(&pos_state_decay_finished_block);
        ctx.b
            .build_store(&position_ptr, &ctx.context.f64_type().const_float(0.));
        ctx.b.build_store(&state_ptr, &state_sustain_const);
        ctx.b.build_unconditional_branch(&active_continue_block);

        ctx.b.position_at_end(&pos_state_release_block);
        let new_pos = ctx.b.build_float_add(
            ctx.b.build_load(&position_ptr, "pos").into_float_value(),
            pos_delta,
            "newpos",
        );
        ctx.b.build_store(&position_ptr, &new_pos);
        let is_release_finished = ctx.b.build_float_compare(
            FloatPredicate::OGE,
            new_pos,
            release_float,
            "release.finished",
        );
        ctx.b.build_conditional_branch(
            &is_release_finished,
            &pos_state_release_finished_block,
            &active_continue_block,
        );

        ctx.b.position_at_end(&pos_state_release_finished_block);
        ctx.b
            .build_store(&position_ptr, &ctx.context.f64_type().const_float(0.));
        ctx.b.build_store(&state_ptr, &state_not_active_const);
        ctx.b.build_unconditional_branch(&active_continue_block);

        ctx.b.position_at_end(&active_continue_block);
        let current_state = ctx.b.build_load(&state_ptr, "state").into_int_value();
        ctx.b.build_switch(
            &current_state,
            &val_state_default_block,
            &[
                (&state_attack_const, &val_state_attack_block),
                (&state_decay_const, &val_state_decay_block),
                (&state_sustain_const, &val_state_sustain_block),
                (&state_release_const, &val_state_release_block),
            ],
        );

        ctx.b.position_at_end(&val_state_attack_block);
        let new_val = ctx.b.build_float_div(
            ctx.b.build_load(&position_ptr, "").into_float_value(),
            attack_float,
            "",
        );
        ctx.b.build_store(&last_val_ptr, &new_val);
        ctx.b.build_unconditional_branch(&val_state_continue_block);

        ctx.b.position_at_end(&val_state_decay_block);
        // f = 1. - pow(1. - pos / decay, 2.)
        let f = ctx.b.build_float_sub(
            ctx.context.f64_type().const_float(1.),
            ctx.b
                .build_extract_element(
                    &ctx.b
                        .build_call(
                            &pow_intrinsic,
                            &[
                                &ctx.b.build_insert_element(
                                    &ctx.context.f64_type().vec_type(2).get_undef(),
                                    &ctx.b.build_float_sub(
                                        ctx.context.f64_type().const_float(1.),
                                        ctx.b.build_float_div(
                                            ctx.b.build_load(&position_ptr, "").into_float_value(),
                                            decay_float,
                                            "",
                                        ),
                                        "",
                                    ),
                                    &ctx.context.i64_type().const_int(0, false),
                                    "",
                                ),
                                &util::get_vec_spread(ctx.context, 2.),
                            ],
                            "",
                            true,
                        )
                        .left()
                        .unwrap()
                        .into_vector_value(),
                    &ctx.context.i64_type().const_int(0, false),
                    "",
                )
                .into_float_value(),
            "f",
        );
        let new_val = ctx.b.build_float_add(
            ctx.b.build_float_mul(
                ctx.context.f64_type().const_float(1.),
                ctx.b
                    .build_float_sub(ctx.context.f64_type().const_float(1.), f, ""),
                "",
            ),
            ctx.b.build_float_mul(sustain_float, f, ""),
            "",
        );
        ctx.b.build_store(&last_val_ptr, &new_val);
        ctx.b.build_unconditional_branch(&val_state_continue_block);

        ctx.b.position_at_end(&val_state_sustain_block);
        ctx.b.build_store(&last_val_ptr, &sustain_float);
        ctx.b.build_unconditional_branch(&val_state_continue_block);

        ctx.b.position_at_end(&val_state_release_block);
        // f = 1. - pow(1. - pos / release, 2.)
        let f = ctx.b.build_float_sub(
            ctx.context.f64_type().const_float(1.),
            ctx.b
                .build_extract_element(
                    &ctx.b
                        .build_call(
                            &pow_intrinsic,
                            &[
                                &ctx.b.build_insert_element(
                                    &ctx.context.f64_type().vec_type(2).get_undef(),
                                    &ctx.b.build_float_sub(
                                        ctx.context.f64_type().const_float(1.),
                                        ctx.b.build_float_div(
                                            ctx.b.build_load(&position_ptr, "").into_float_value(),
                                            release_float,
                                            "",
                                        ),
                                        "",
                                    ),
                                    &ctx.context.i64_type().const_int(0, false),
                                    "",
                                ),
                                &util::get_vec_spread(ctx.context, 2.),
                            ],
                            "",
                            true,
                        )
                        .left()
                        .unwrap()
                        .into_vector_value(),
                    &ctx.context.i64_type().const_int(0, false),
                    "",
                )
                .into_float_value(),
            "f",
        );
        let new_val = ctx.b.build_float_mul(
            ctx.b.build_load(&release_val_ptr, "").into_float_value(),
            ctx.b
                .build_float_sub(ctx.context.f64_type().const_float(1.), f, ""),
            "",
        );
        ctx.b.build_store(&last_val_ptr, &new_val);
        ctx.b.build_unconditional_branch(&val_state_continue_block);

        ctx.b.position_at_end(&val_state_default_block);
        ctx.b
            .build_store(&last_val_ptr, &ctx.context.f64_type().const_float(0.));
        ctx.b.build_unconditional_branch(&val_state_continue_block);

        ctx.b.position_at_end(&val_state_continue_block);

        // Build the result struct
        let is_active = ctx.b.build_int_compare(
            IntPredicate::NE,
            ctx.b.build_load(&state_ptr, "").into_int_value(),
            state_not_active_const,
            "",
        );
        let result_num = ctx.b.build_load(&last_val_ptr, "").into_float_value();

        ctx.b.build_aggregate_return(&[&is_active, &result_num]);
    })
}

impl Function for AdsrFunction {
    fn function_type() -> block::Function {
        block::Function::Adsr
    }

    fn data_type(context: &Context) -> StructType {
        let channel_update_type = get_channel_update_data_type(context);
        context.struct_type(&[&channel_update_type, &channel_update_type], false)
    }

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        _varargs: Option<VarArgs>,
        result: PointerValue,
    ) {
        build_channel_update_func(func.ctx.module, func.ctx.target);
        let channel_update_func = get_channel_update_func(func.ctx.module);

        let left_update_data =
            unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 0, "data.left") };
        let right_update_data =
            unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 1, "data.right") };

        let trig_num = NumValue::new(args[0]);
        let attack_num = NumValue::new(args[1]);
        let decay_num = NumValue::new(args[2]);
        let sustain_num = NumValue::new(args[3]);
        let release_num = NumValue::new(args[4]);

        let result_tuple = TupleValue::new(result);
        let result_active = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 0));
        let result_num = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 1));

        let trig_vec = trig_num.get_vec(func.ctx.b);
        let attack_vec = attack_num.get_vec(func.ctx.b);
        let decay_vec = decay_num.get_vec(func.ctx.b);
        let sustain_vec = sustain_num.get_vec(func.ctx.b);
        let release_vec = release_num.get_vec(func.ctx.b);

        let trig_bool = func.ctx.b.build_float_compare(
            FloatPredicate::ONE,
            trig_vec,
            util::get_vec_spread(func.ctx.context, 0.),
            "",
        );

        // calculate result for the left channel
        let left_index = func.ctx.context.i64_type().const_int(0, false);
        let left_trigger = func
            .ctx
            .b
            .build_extract_element(&trig_bool, &left_index, "trig.left")
            .into_int_value();
        let left_attack = func
            .ctx
            .b
            .build_extract_element(&attack_vec, &left_index, "attack.left")
            .into_float_value();
        let left_decay = func
            .ctx
            .b
            .build_extract_element(&decay_vec, &left_index, "decay.left")
            .into_float_value();
        let left_sustain = func
            .ctx
            .b
            .build_extract_element(&sustain_vec, &left_index, "sustain.left")
            .into_float_value();
        let left_release = func
            .ctx
            .b
            .build_extract_element(&release_vec, &left_index, "release.left")
            .into_float_value();
        let left_result = func
            .ctx
            .b
            .build_call(
                &channel_update_func,
                &[
                    &left_update_data,
                    &left_trigger,
                    &left_attack,
                    &left_decay,
                    &left_sustain,
                    &left_release,
                ],
                "",
                true,
            )
            .left()
            .unwrap()
            .into_struct_value();

        // calculate result for the right channel
        let right_index = func.ctx.context.i64_type().const_int(1, false);
        let right_trigger = func
            .ctx
            .b
            .build_extract_element(&trig_bool, &right_index, "trig.right")
            .into_int_value();
        let right_attack = func
            .ctx
            .b
            .build_extract_element(&attack_vec, &right_index, "attack.right")
            .into_float_value();
        let right_decay = func
            .ctx
            .b
            .build_extract_element(&decay_vec, &right_index, "decay.right")
            .into_float_value();
        let right_sustain = func
            .ctx
            .b
            .build_extract_element(&sustain_vec, &right_index, "sustain.right")
            .into_float_value();
        let right_release = func
            .ctx
            .b
            .build_extract_element(&release_vec, &right_index, "release.right")
            .into_float_value();
        let right_result = func
            .ctx
            .b
            .build_call(
                &channel_update_func,
                &[
                    &right_update_data,
                    &right_trigger,
                    &right_attack,
                    &right_decay,
                    &right_sustain,
                    &right_release,
                ],
                "",
                true,
            )
            .left()
            .unwrap()
            .into_struct_value();

        let active_vec = func
            .ctx
            .b
            .build_insert_element(
                &func
                    .ctx
                    .b
                    .build_insert_element(
                        &func.ctx.context.f64_type().vec_type(2).get_undef(),
                        &func.ctx.b.build_unsigned_int_to_float(
                            func.ctx
                                .b
                                .build_extract_value(left_result, 0, "left.active")
                                .into_int_value(),
                            func.ctx.context.f64_type(),
                            "",
                        ),
                        &left_index,
                        "",
                    )
                    .into_vector_value(),
                &func.ctx.b.build_unsigned_int_to_float(
                    func.ctx
                        .b
                        .build_extract_value(right_result, 0, "right.active")
                        .into_int_value(),
                    func.ctx.context.f64_type(),
                    "",
                ),
                &right_index,
                "",
            )
            .into_vector_value();
        result_active.set_vec(func.ctx.b, active_vec);
        result_active.set_form(
            func.ctx.b,
            func.ctx
                .context
                .i8_type()
                .const_int(FormType::None as u64, false),
        );

        // put the results in the return value
        let result_vec = func
            .ctx
            .b
            .build_insert_element(
                &func
                    .ctx
                    .b
                    .build_insert_element(
                        &func.ctx.context.f64_type().vec_type(2).get_undef(),
                        &func
                            .ctx
                            .b
                            .build_extract_value(left_result, 1, "left.value")
                            .into_float_value(),
                        &left_index,
                        "",
                    )
                    .into_vector_value(),
                &func
                    .ctx
                    .b
                    .build_extract_value(right_result, 1, "right.value")
                    .into_float_value(),
                &right_index,
                "",
            )
            .into_vector_value();
        result_num.set_vec(func.ctx.b, result_vec);
        result_num.set_form(
            func.ctx.b,
            func.ctx
                .context
                .i8_type()
                .const_int(FormType::Amplitude as u64, false),
        );
    }
}
