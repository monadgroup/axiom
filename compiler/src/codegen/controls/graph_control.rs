use super::ControlContext;
use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use ast::{ControlField, ControlType, FormType, GraphField};
use codegen::values::NumValue;
use codegen::{
    build_context_function, globals, intrinsics, util, BuilderContext, TargetProperties,
};
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::StructType;
use inkwell::values::{FunctionValue, PointerValue};
use inkwell::{FloatPredicate, IntPredicate};

pub struct GraphControl;
impl GraphControl {
    fn get_tension_graph_func(module: &Module) -> FunctionValue {
        util::get_or_create_func(module, "maxim.util.graph.tensionGraph", true, &|| {
            let context = &module.get_context();
            (
                Linkage::PrivateLinkage,
                context
                    .f32_type()
                    .fn_type(&[&context.f32_type(), &context.f32_type()], false),
            )
        })
    }

    /// Builds a function that is equivalent to the following C++:
    /// ```cpp
    /// float tensionGraph(float x, float tension) {
    ///     const float q = 20;
    ///     if (tension >= 0) {
    ///         return powf(x, powf(q, tension));
    ///     } else {
    ///         return 1 - powf(1 - x, powf(q, -tension));
    ///     }
    /// }
    /// ```
    fn build_tension_graph_func(module: &Module, target: &TargetProperties) {
        let func = GraphControl::get_tension_graph_func(module);
        build_context_function(module, func, target, &|ctx: BuilderContext| {
            let pow_intrinsic = intrinsics::pow_f32(ctx.module);

            let q_value = ctx.context.f32_type().const_float(20.);

            let tension_positive_true_block = ctx
                .context
                .append_basic_block(&ctx.func, "tensionpositive.true");
            let tension_positive_false_block = ctx
                .context
                .append_basic_block(&ctx.func, "tensionpositive.false");

            let x = ctx.func.get_nth_param(0).unwrap().into_float_value();
            let tension = ctx.func.get_nth_param(1).unwrap().into_float_value();

            let tension_positive = ctx.b.build_float_compare(
                FloatPredicate::OGE,
                tension,
                ctx.context.f32_type().const_float(0.),
                "tensionpositive",
            );
            ctx.b.build_conditional_branch(
                &tension_positive,
                &tension_positive_true_block,
                &tension_positive_false_block,
            );

            ctx.b.position_at_end(&tension_positive_true_block);
            ctx.b.build_return(Some(
                &ctx.b
                    .build_call(
                        &pow_intrinsic,
                        &[
                            &x,
                            &ctx.b
                                .build_call(&pow_intrinsic, &[&q_value, &tension], "", false)
                                .left()
                                .unwrap()
                                .into_float_value(),
                        ],
                        "",
                        false,
                    ).left()
                    .unwrap()
                    .into_float_value(),
            ));

            ctx.b.position_at_end(&tension_positive_false_block);
            let one_const = ctx.context.f32_type().const_float(1.);
            ctx.b.build_return(Some(
                &ctx.b.build_float_sub(
                    one_const,
                    ctx.b
                        .build_call(
                            &pow_intrinsic,
                            &[
                                &ctx.b.build_float_sub(one_const, x, ""),
                                &ctx.b
                                    .build_call(
                                        &pow_intrinsic,
                                        &[&q_value, &ctx.b.build_float_neg(&tension, "")],
                                        "",
                                        false,
                                    ).left()
                                    .unwrap()
                                    .into_float_value(),
                            ],
                            "",
                            false,
                        ).left()
                        .unwrap()
                        .into_float_value(),
                    "",
                ),
            ));
        });
    }
}

impl Control for GraphControl {
    fn control_type() -> ControlType {
        ControlType::Graph
    }

    fn data_type(context: &Context) -> StructType {
        context.struct_type(
            &[
                &context.i32_type(),  // current time
                &context.i8_type(),   // current state
                &context.bool_type(), // paused?
            ],
            false,
        )
    }

    fn shared_data_type(context: &Context) -> StructType {
        context.struct_type(
            &[
                &context.i8_type(),                 // curve count
                &context.f32_type().array_type(17), // start values
                &context.f32_type().array_type(16), // end positions
                &context.f32_type().array_type(16), // tension
                &context.i8_type().array_type(17),  // states
            ],
            false,
        )
    }

    fn gen_update(control: &mut ControlContext) {
        GraphControl::build_tension_graph_func(control.ctx.module, control.ctx.target);
        let tension_graph_func = GraphControl::get_tension_graph_func(control.ctx.module);

        let current_time_samples_ptr = unsafe {
            control
                .ctx
                .b
                .build_struct_gep(&control.data_ptr, 0, "currentsamples.ptr")
        };
        let current_time_samples = control
            .ctx
            .b
            .build_load(&current_time_samples_ptr, "currentsamples")
            .into_int_value();
        let current_state = control
            .ctx
            .b
            .build_load(
                &unsafe { control.ctx.b.build_struct_gep(&control.data_ptr, 1, "") },
                "currentstate",
            ).into_int_value();
        let is_paused_ptr = unsafe {
            control
                .ctx
                .b
                .build_struct_gep(&control.data_ptr, 2, "paused.ptr")
        };
        let curve_count = control
            .ctx
            .b
            .build_load(
                &unsafe { control.ctx.b.build_struct_gep(&control.shared_ptr, 0, "") },
                "curvecount",
            ).into_int_value();

        let start_vals_array_ptr =
            unsafe { control.ctx.b.build_struct_gep(&control.shared_ptr, 1, "") };
        let end_positions_array_ptr =
            unsafe { control.ctx.b.build_struct_gep(&control.shared_ptr, 2, "") };
        let tension_array_ptr =
            unsafe { control.ctx.b.build_struct_gep(&control.shared_ptr, 3, "") };
        let state_array_ptr = unsafe { control.ctx.b.build_struct_gep(&control.shared_ptr, 4, "") };

        let samplerate = control
            .ctx
            .b
            .build_extract_element(
                &control
                    .ctx
                    .b
                    .build_load(
                        &globals::get_sample_rate(control.ctx.module).as_pointer_value(),
                        "samplerate",
                    ).into_vector_value(),
                &control.ctx.context.i64_type().const_int(0, false),
                "",
            ).into_float_value();
        let samplerate_beats = control.ctx.b.build_float_mul(
            samplerate,
            control.ctx.context.f32_type().const_float(60.),
            "",
        );
        let bpm = control
            .ctx
            .b
            .build_extract_element(
                &control
                    .ctx
                    .b
                    .build_load(
                        &globals::get_bpm(control.ctx.module).as_pointer_value(),
                        "bpm",
                    ).into_vector_value(),
                &control.ctx.context.i64_type().const_int(0, false),
                "",
            ).into_float_value();

        // build a loop to look through all the curves and find the one we're currently in
        let curve_loop_index_ptr = control
            .ctx
            .allocb
            .build_alloca(&control.ctx.context.i8_type(), "curveindex.ptr");
        control.ctx.b.build_store(
            &curve_loop_index_ptr,
            &control.ctx.context.i8_type().const_int(0, false),
        );
        let last_curve_end_samples_ptr = control
            .ctx
            .allocb
            .build_alloca(&control.ctx.context.i32_type(), "lastcurveend.ptr");
        control.ctx.b.build_store(
            &last_curve_end_samples_ptr,
            &control.ctx.context.i32_type().const_int(0, false),
        );

        let loop_check_block = control
            .ctx
            .context
            .append_basic_block(&control.ctx.func, "loopcheck");
        let loop_body_block = control
            .ctx
            .context
            .append_basic_block(&control.ctx.func, "loopbody");
        let curve_active_true_block = control
            .ctx
            .context
            .append_basic_block(&control.ctx.func, "curveactive.true");
        let curve_has_length_true_block = control
            .ctx
            .context
            .append_basic_block(&control.ctx.func, "curvelength.true");
        let curve_has_length_false_block = control
            .ctx
            .context
            .append_basic_block(&control.ctx.func, "curvelength.false");
        let curve_has_length_continue_block = control
            .ctx
            .context
            .append_basic_block(&control.ctx.func, "curvelength.continue");
        let increment_sample_true_block = control
            .ctx
            .context
            .append_basic_block(&control.ctx.func, "incrementsample.true");
        let curve_active_false_block = control
            .ctx
            .context
            .append_basic_block(&control.ctx.func, "curveactive.false");
        let loop_end_block = control
            .ctx
            .context
            .append_basic_block(&control.ctx.func, "loopend");
        let loop_after_block = control
            .ctx
            .context
            .append_basic_block(&control.ctx.func, "loopafter");

        control.ctx.b.build_unconditional_branch(&loop_check_block);
        control.ctx.b.position_at_end(&loop_check_block);
        let current_loop_index = control
            .ctx
            .b
            .build_load(&curve_loop_index_ptr, "curveindex")
            .into_int_value();
        let continue_cond = control.ctx.b.build_int_compare(
            IntPredicate::ULT,
            current_loop_index,
            curve_count,
            "continue.cond",
        );
        control
            .ctx
            .b
            .build_conditional_branch(&continue_cond, &loop_body_block, &loop_end_block);

        control.ctx.b.position_at_end(&loop_body_block);
        let next_loop_index = control.ctx.b.build_int_add(
            current_loop_index,
            control.ctx.context.i8_type().const_int(1, false),
            "",
        );
        control
            .ctx
            .b
            .build_store(&curve_loop_index_ptr, &next_loop_index);

        let curve_end_beats = control
            .ctx
            .b
            .build_load(
                &unsafe {
                    control.ctx.b.build_in_bounds_gep(
                        &end_positions_array_ptr,
                        &[
                            control.ctx.context.i64_type().const_int(0, false),
                            current_loop_index,
                        ],
                        "",
                    )
                },
                "curveend.beats",
            ).into_float_value();

        // convert the beats into samples
        let curve_end_samples = control.ctx.b.build_float_to_unsigned_int(
            control.ctx.b.build_float_div(
                control
                    .ctx
                    .b
                    .build_float_mul(curve_end_beats, samplerate_beats, ""),
                bpm,
                "",
            ),
            control.ctx.context.i32_type(),
            "curveend.samples",
        );

        let curve_active_cond = control.ctx.b.build_int_compare(
            IntPredicate::ULT,
            current_time_samples,
            curve_end_samples,
            "curveactive.cond",
        );

        // determine if the graph is paused at the curves start - we must pause here even if the
        // curve isn't active - this happens if the curve is zero-length but still tagged
        let curve_state = control
            .ctx
            .b
            .build_load(
                &unsafe {
                    control.ctx.b.build_in_bounds_gep(
                        &state_array_ptr,
                        &[
                            control.ctx.context.i64_type().const_int(0, false),
                            current_loop_index,
                        ],
                        "curvestate.ptr",
                    )
                },
                "curvestate",
            ).into_int_value();
        let last_curve_end_samples = control
            .ctx
            .b
            .build_load(&last_curve_end_samples_ptr, "lastcurveend")
            .into_int_value();
        let increment_sample = control.ctx.b.build_or(
            control.ctx.b.build_int_compare(
                IntPredicate::UGT,
                current_time_samples,
                last_curve_end_samples,
                "",
            ),
            control.ctx.b.build_int_compare(
                IntPredicate::NE,
                curve_state,
                control.ctx.b.build_int_add(
                    current_state,
                    control.ctx.context.i8_type().const_int(1, false),
                    "",
                ),
                "",
            ),
            "",
        );
        let is_paused = control.ctx.b.build_not(&increment_sample, "paused");

        let curve_active_or_paused =
            control
                .ctx
                .b
                .build_or(curve_active_cond, is_paused, "activeorpaused.cond");

        control.ctx.b.build_conditional_branch(
            &curve_active_or_paused,
            &curve_active_true_block,
            &curve_active_false_block,
        );

        control.ctx.b.position_at_end(&curve_active_true_block);

        // calculate current output in this graph
        let sample_offset = control.ctx.b.build_int_sub(
            current_time_samples,
            last_curve_end_samples,
            "sampleoffset",
        );
        let curve_duration_int =
            control
                .ctx
                .b
                .build_int_sub(curve_end_samples, last_curve_end_samples, "curveduration");
        let curve_min_value = control
            .ctx
            .b
            .build_load(
                &unsafe {
                    control.ctx.b.build_in_bounds_gep(
                        &start_vals_array_ptr,
                        &[
                            control.ctx.context.i64_type().const_int(0, false),
                            current_loop_index,
                        ],
                        "curvemin.ptr",
                    )
                },
                "curvemin",
            ).into_float_value();
        let curve_has_duration = control.ctx.b.build_int_compare(
            IntPredicate::NE,
            curve_duration_int,
            control.ctx.context.i32_type().const_int(0, false),
            "curveduration.cond",
        );
        control.ctx.b.build_conditional_branch(
            &curve_has_duration,
            &curve_has_length_true_block,
            &curve_has_length_false_block,
        );

        let output_float_ptr = control
            .ctx
            .allocb
            .build_alloca(&control.ctx.context.f32_type(), "output.float.ptr");

        control.ctx.b.position_at_end(&curve_has_length_true_block);
        let curve_function_x = control.ctx.b.build_float_div(
            control.ctx.b.build_unsigned_int_to_float(
                sample_offset,
                control.ctx.context.f32_type(),
                "currenttime.float",
            ),
            control.ctx.b.build_unsigned_int_to_float(
                curve_duration_int,
                control.ctx.context.f32_type(),
                "curveduration.float",
            ),
            "curve.x",
        );
        let current_tension = control
            .ctx
            .b
            .build_load(
                &unsafe {
                    control.ctx.b.build_in_bounds_gep(
                        &tension_array_ptr,
                        &[
                            control.ctx.context.i64_type().const_int(0, false),
                            current_loop_index,
                        ],
                        "tension.ptr",
                    )
                },
                "tension",
            ).into_float_value();
        let curve_function_y = control
            .ctx
            .b
            .build_call(
                &tension_graph_func,
                &[&curve_function_x, &current_tension],
                "curve.y",
                true,
            ).left()
            .unwrap()
            .into_float_value();
        let curve_max_value = control
            .ctx
            .b
            .build_load(
                &unsafe {
                    control.ctx.b.build_in_bounds_gep(
                        &start_vals_array_ptr,
                        &[
                            control.ctx.context.i64_type().const_int(0, false),
                            next_loop_index,
                        ],
                        "curvemax.ptr",
                    )
                },
                "curvemax",
            ).into_float_value();

        // mix between the min and max values to get the output value
        let output_float = control.ctx.b.build_float_add(
            curve_min_value,
            control.ctx.b.build_float_mul(
                control
                    .ctx
                    .b
                    .build_float_sub(curve_max_value, curve_min_value, ""),
                curve_function_y,
                "",
            ),
            "output.float",
        );
        control.ctx.b.build_store(&output_float_ptr, &output_float);
        control
            .ctx
            .b
            .build_unconditional_branch(&curve_has_length_continue_block);

        control.ctx.b.position_at_end(&curve_has_length_false_block);
        control
            .ctx
            .b
            .build_store(&output_float_ptr, &curve_min_value);
        control
            .ctx
            .b
            .build_unconditional_branch(&curve_has_length_continue_block);

        control
            .ctx
            .b
            .position_at_end(&curve_has_length_continue_block);
        let output_float = control
            .ctx
            .b
            .build_load(&output_float_ptr, "output.float")
            .into_float_value();
        let output_vec = util::splat_vector(control.ctx.b, output_float, "output.vec");
        let output_num = NumValue::new(control.val_ptr);
        output_num.set_vec(control.ctx.b, &output_vec);
        output_num.set_form(
            control.ctx.b,
            &control
                .ctx
                .context
                .i8_type()
                .const_int(FormType::None as u64, false),
        );

        // increment time if not at the start of the curve OR state == input state
        control.ctx.b.build_store(&is_paused_ptr, &is_paused);
        control.ctx.b.build_conditional_branch(
            &increment_sample,
            &increment_sample_true_block,
            &loop_after_block,
        );

        control.ctx.b.position_at_end(&increment_sample_true_block);
        let new_samples_count = control.ctx.b.build_int_add(
            current_time_samples,
            control.ctx.context.i32_type().const_int(1, false),
            "newsamples",
        );
        control
            .ctx
            .b
            .build_store(&current_time_samples_ptr, &new_samples_count);
        control.ctx.b.build_unconditional_branch(&loop_after_block);

        control.ctx.b.position_at_end(&curve_active_false_block);
        control
            .ctx
            .b
            .build_store(&last_curve_end_samples_ptr, &curve_end_samples);
        control.ctx.b.build_unconditional_branch(&loop_check_block);

        control.ctx.b.position_at_end(&loop_end_block);
        control.ctx.b.build_store(
            &current_time_samples_ptr,
            &control.ctx.context.i32_type().const_int(0, false),
        );
        control.ctx.b.build_unconditional_branch(&loop_after_block);

        control.ctx.b.position_at_end(&loop_after_block);
    }

    fn gen_fields(generator: &ControlFieldGenerator) {
        generator.generate(
            ControlField::Graph(GraphField::Value),
            &default_copy_getter,
            &default_copy_setter,
        );
        generator.generate(
            ControlField::Graph(GraphField::State),
            &state_field_getter,
            &state_field_setter,
        );
        generator.generate(
            ControlField::Graph(GraphField::Paused),
            &paused_field_getter,
            &paused_field_setter,
        );
        generator.generate(
            ControlField::Graph(GraphField::Time),
            &time_field_getter,
            &time_field_setter,
        )
    }
}

fn state_field_getter(control: &mut ControlContext, out_val: PointerValue) {
    let state_int = control
        .ctx
        .b
        .build_load(
            &unsafe { control.ctx.b.build_struct_gep(&control.data_ptr, 1, "") },
            "",
        ).into_int_value();
    let state_float =
        control
            .ctx
            .b
            .build_unsigned_int_to_float(state_int, control.ctx.context.f32_type(), "");

    let result_num = NumValue::new(out_val);
    let result_vector = util::splat_vector(control.ctx.b, state_float, "");
    result_num.set_vec(control.ctx.b, &result_vector);
    result_num.set_form(
        control.ctx.b,
        &control
            .ctx
            .context
            .i8_type()
            .const_int(FormType::None as u64, false),
    );
}

fn state_field_setter(control: &mut ControlContext, in_val: PointerValue) {
    let min_intrinsic = intrinsics::minnum_f32(control.ctx.module);
    let max_intrinsic = intrinsics::maxnum_f32(control.ctx.module);

    let in_num = NumValue::new(in_val);

    // take the left component and clamp it between {0, MAX_U8 - 1}
    let in_vec = in_num.get_vec(control.ctx.b);
    let left_float = control
        .ctx
        .b
        .build_extract_element(
            &in_vec,
            &control.ctx.context.i32_type().const_int(0, false),
            "",
        ).into_float_value();
    let clamped_left = control
        .ctx
        .b
        .build_call(
            &max_intrinsic,
            &[
                &control.ctx.context.f32_type().const_float(0.),
                &control
                    .ctx
                    .b
                    .build_call(
                        &min_intrinsic,
                        &[
                            &control
                                .ctx
                                .context
                                .f32_type()
                                .const_float(<u8>::max_value() as f64 - 1.),
                            &left_float,
                        ],
                        "",
                        false,
                    ).left()
                    .unwrap()
                    .into_float_value(),
            ],
            "",
            false,
        ).left()
        .unwrap()
        .into_float_value();

    let state_int =
        control
            .ctx
            .b
            .build_float_to_unsigned_int(clamped_left, control.ctx.context.i8_type(), "");
    control.ctx.b.build_store(
        &unsafe { control.ctx.b.build_struct_gep(&control.data_ptr, 1, "") },
        &state_int,
    );
}

fn paused_field_getter(control: &mut ControlContext, out_val: PointerValue) {
    let out_num = NumValue::new(out_val);

    let is_paused_int = control
        .ctx
        .b
        .build_load(
            &unsafe {
                control
                    .ctx
                    .b
                    .build_struct_gep(&control.data_ptr, 2, "paused.int.ptr")
            },
            "paused.int",
        ).into_int_value();
    let is_paused_float = control.ctx.b.build_unsigned_int_to_float(
        is_paused_int,
        control.ctx.context.f32_type(),
        "paused.float",
    );
    let is_paused_spread = util::splat_vector(&control.ctx.b, is_paused_float, "paused.splat");
    out_num.set_vec(control.ctx.b, &is_paused_spread);
    out_num.set_form(
        control.ctx.b,
        &control
            .ctx
            .context
            .i8_type()
            .const_int(FormType::None as u64, false),
    );
}

fn paused_field_setter(_control: &mut ControlContext, _in_val: PointerValue) {
    // noop
}

fn time_field_getter(control: &mut ControlContext, out_val: PointerValue) {
    let out_num = NumValue::new(out_val);

    let current_time_int = control
        .ctx
        .b
        .build_load(
            &unsafe {
                control
                    .ctx
                    .b
                    .build_struct_gep(&control.data_ptr, 0, "time.int.ptr")
            },
            "time.int",
        ).into_int_value();
    let current_time_float = control.ctx.b.build_unsigned_int_to_float(
        current_time_int,
        control.ctx.context.f32_type(),
        "time.float",
    );
    let time_spread = util::splat_vector(&control.ctx.b, current_time_float, "time.splat");
    out_num.set_vec(control.ctx.b, &time_spread);
    out_num.set_form(
        control.ctx.b,
        &control
            .ctx
            .context
            .i8_type()
            .const_int(FormType::Samples as u64, false),
    );
}

fn time_field_setter(control: &mut ControlContext, in_val: PointerValue) {
    let max_intrinsic = intrinsics::maxnum_f32(control.ctx.module);

    let in_num = NumValue::new(in_val);
    let in_vec = in_num.get_vec(control.ctx.b);
    let new_time_float = control
        .ctx
        .b
        .build_extract_element(
            &in_vec,
            &control.ctx.context.i64_type().const_int(0, false),
            "time.float",
        ).into_float_value();
    let clamped_time = control
        .ctx
        .b
        .build_call(
            &max_intrinsic,
            &[
                &new_time_float,
                &control.ctx.context.f32_type().const_float(0.),
            ],
            "",
            false,
        ).left()
        .unwrap()
        .into_float_value();

    let int_time = control.ctx.b.build_float_to_unsigned_int(
        clamped_time,
        control.ctx.context.i32_type(),
        "time.int",
    );
    control.ctx.b.build_store(
        &unsafe {
            control
                .ctx
                .b
                .build_struct_gep(&control.data_ptr, 0, "time.int.ptr")
        },
        &int_time,
    );
}
