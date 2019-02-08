use super::ControlContext;
use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use crate::ast::{ControlField, ControlType, FormType, GraphField};
use crate::codegen::data_analyzer::PointerSource;
use crate::codegen::values::NumValue;
use crate::codegen::{
    build_context_function, globals, math, util, BuilderContext, TargetProperties,
};
use crate::mir::ControlInitializer;
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::StructType;
use inkwell::values::{FunctionValue, PointerValue, StructValue};
use inkwell::{AddressSpace, FloatPredicate, IntPredicate};
use std::iter;

pub struct GraphControl;
impl GraphControl {
    fn get_tension_graph_func(module: &Module) -> FunctionValue {
        util::get_or_create_func(module, "maxim.util.graph.tensionGraph", true, &|| {
            let context = &module.get_context();
            (
                Linkage::PrivateLinkage,
                context
                    .f64_type()
                    .fn_type(&[&context.f64_type(), &context.f64_type()], false),
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
            let pow_intrinsic = math::pow_v2f64(ctx.module);

            let q_value = ctx.context.f64_type().const_float(20.);

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
                ctx.context.f64_type().const_float(0.),
                "tensionpositive",
            );
            ctx.b.build_conditional_branch(
                &tension_positive,
                &tension_positive_true_block,
                &tension_positive_false_block,
            );

            ctx.b.position_at_end(&tension_positive_true_block);
            let undef_vec = ctx.context.f64_type().vec_type(2).get_undef();
            let zero_index = ctx.context.i32_type().const_int(0, false);
            let first_pow = ctx
                .b
                .build_call(
                    &pow_intrinsic,
                    &[
                        &ctx.b
                            .build_insert_element(&undef_vec, &q_value, &zero_index, ""),
                        &ctx.b
                            .build_insert_element(&undef_vec, &tension, &zero_index, ""),
                    ],
                    "",
                    true,
                )
                .left()
                .unwrap()
                .into_vector_value();
            let second_pow = ctx
                .b
                .build_call(
                    &pow_intrinsic,
                    &[
                        &ctx.b.build_insert_element(&undef_vec, &x, &zero_index, ""),
                        &first_pow,
                    ],
                    "",
                    true,
                )
                .left()
                .unwrap()
                .into_vector_value();

            ctx.b.build_return(Some(
                &ctx.b
                    .build_extract_element(&second_pow, &zero_index, "")
                    .into_float_value(),
            ));

            ctx.b.position_at_end(&tension_positive_false_block);
            let one_const = ctx.context.f64_type().const_float(1.);
            let first_pow = ctx
                .b
                .build_call(
                    &pow_intrinsic,
                    &[
                        &ctx.b
                            .build_insert_element(&undef_vec, &q_value, &zero_index, ""),
                        &ctx.b.build_insert_element(
                            &undef_vec,
                            &ctx.b.build_float_neg(&tension, ""),
                            &zero_index,
                            "",
                        ),
                    ],
                    "",
                    true,
                )
                .left()
                .unwrap()
                .into_vector_value();
            let second_pow = ctx
                .b
                .build_call(
                    &pow_intrinsic,
                    &[
                        &ctx.b.build_insert_element(
                            &undef_vec,
                            &ctx.b.build_float_sub(one_const, x, ""),
                            &zero_index,
                            "",
                        ),
                        &first_pow,
                    ],
                    "",
                    true,
                )
                .left()
                .unwrap()
                .into_vector_value();
            ctx.b.build_return(Some(
                &ctx.b.build_float_sub(
                    one_const,
                    ctx.b
                        .build_extract_element(&second_pow, &zero_index, "")
                        .into_float_value(),
                    "",
                ),
            ));
        });
    }
}

fn conditionally_pad<T: Clone>(
    iter: impl Iterator<Item = T>,
    pad_val: T,
    pad_length: usize,
    do_pad: bool,
) -> Vec<T> {
    if do_pad {
        iter.chain(iter::repeat(pad_val)).take(pad_length).collect()
    } else {
        iter.collect()
    }
}

impl Control for GraphControl {
    fn control_type() -> ControlType {
        ControlType::Graph
    }

    fn constant_ptr_type(context: &Context) -> StructType {
        context.struct_type(
            &[
                &context.i8_type().ptr_type(AddressSpace::Generic), // curve count
                &context.f64_type().ptr_type(AddressSpace::Generic), // start values array
                &context.f64_type().ptr_type(AddressSpace::Generic), // end positions array
                &context.f64_type().ptr_type(AddressSpace::Generic), // tension array
                &context.i8_type().ptr_type(AddressSpace::Generic), // states array
            ],
            false,
        )
    }

    fn constant_value(
        context: &Context,
        initializer: &ControlInitializer,
        target: &TargetProperties,
    ) -> (StructValue, Vec<PointerSource>) {
        let graph_ini = initializer.as_graph_control().unwrap();

        // Don't bother including the extra undefined values if they're not editable
        let start_value_consts: Vec<_> = conditionally_pad(
            graph_ini
                .start_values
                .iter()
                .map(|&val| context.f64_type().const_float(val)),
            context.f64_type().get_undef(),
            17,
            target.include_ui,
        );
        let end_pos_consts: Vec<_> = conditionally_pad(
            graph_ini
                .end_positions
                .iter()
                .map(|&val| context.f64_type().const_float(val)),
            context.f64_type().get_undef(),
            16,
            target.include_ui,
        );
        let tension_consts: Vec<_> = conditionally_pad(
            graph_ini
                .tension
                .iter()
                .map(|&val| context.f64_type().const_float(val)),
            context.f64_type().get_undef(),
            16,
            target.include_ui,
        );
        let state_consts: Vec<_> = conditionally_pad(
            graph_ini
                .states
                .iter()
                .map(|&val| context.i8_type().const_int(val as u64, false)),
            context.i8_type().get_undef(),
            17,
            target.include_ui,
        );

        let initializer_struct = context.const_struct(
            &[
                &context
                    .i8_type()
                    .const_int(graph_ini.curve_count as u64, false),
                &context.f64_type().const_array(&start_value_consts),
                &context.f64_type().const_array(&end_pos_consts),
                &context.f64_type().const_array(&tension_consts),
                &context.i8_type().const_array(&state_consts),
            ],
            false,
        );
        let pointer_sources = vec![
            PointerSource::Initialized(vec![0]),
            PointerSource::Initialized(vec![1, 0]),
            PointerSource::Initialized(vec![2, 0]),
            PointerSource::Initialized(vec![3, 0]),
            PointerSource::Initialized(vec![4, 0]),
        ];

        (initializer_struct, pointer_sources)
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
            )
            .into_int_value();
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
                &control
                    .ctx
                    .b
                    .build_load(
                        &unsafe { control.ctx.b.build_struct_gep(&control.const_ptr, 0, "") },
                        "curvecount.ptr",
                    )
                    .into_pointer_value(),
                "curvecount",
            )
            .into_int_value();

        let start_vals_array_ptr = control
            .ctx
            .b
            .build_load(
                &unsafe { control.ctx.b.build_struct_gep(&control.const_ptr, 1, "") },
                "",
            )
            .into_pointer_value();
        let end_positions_array_ptr = control
            .ctx
            .b
            .build_load(
                &unsafe { control.ctx.b.build_struct_gep(&control.const_ptr, 2, "") },
                "",
            )
            .into_pointer_value();
        let tension_array_ptr = control
            .ctx
            .b
            .build_load(
                &unsafe { control.ctx.b.build_struct_gep(&control.const_ptr, 3, "") },
                "",
            )
            .into_pointer_value();
        let state_array_ptr = control
            .ctx
            .b
            .build_load(
                &unsafe { control.ctx.b.build_struct_gep(&control.const_ptr, 4, "") },
                "",
            )
            .into_pointer_value();

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
                    )
                    .into_vector_value(),
                &control.ctx.context.i64_type().const_int(0, false),
                "",
            )
            .into_float_value();
        let samplerate_beats = control.ctx.b.build_float_mul(
            samplerate,
            control.ctx.context.f64_type().const_float(60.),
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
                    )
                    .into_vector_value(),
                &control.ctx.context.i64_type().const_int(0, false),
                "",
            )
            .into_float_value();

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
        let next_loop_index = control.ctx.b.build_int_nuw_add(
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
                        &[current_loop_index],
                        "",
                    )
                },
                "curveend.beats",
            )
            .into_float_value();

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
                        &[current_loop_index],
                        "curvestate.ptr",
                    )
                },
                "curvestate",
            )
            .into_int_value();
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
                control.ctx.b.build_int_nuw_add(
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
        let sample_offset = control.ctx.b.build_int_nuw_sub(
            current_time_samples,
            last_curve_end_samples,
            "sampleoffset",
        );
        let curve_duration_int = control.ctx.b.build_int_nuw_sub(
            curve_end_samples,
            last_curve_end_samples,
            "curveduration",
        );
        let curve_min_value = control
            .ctx
            .b
            .build_load(
                &unsafe {
                    control.ctx.b.build_in_bounds_gep(
                        &start_vals_array_ptr,
                        &[current_loop_index],
                        "curvemin.ptr",
                    )
                },
                "curvemin",
            )
            .into_float_value();
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
            .build_alloca(&control.ctx.context.f64_type(), "output.float.ptr");

        control.ctx.b.position_at_end(&curve_has_length_true_block);
        let curve_function_x = control.ctx.b.build_float_div(
            control.ctx.b.build_unsigned_int_to_float(
                sample_offset,
                control.ctx.context.f64_type(),
                "currenttime.float",
            ),
            control.ctx.b.build_unsigned_int_to_float(
                curve_duration_int,
                control.ctx.context.f64_type(),
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
                        &[current_loop_index],
                        "tension.ptr",
                    )
                },
                "tension",
            )
            .into_float_value();
        let curve_function_y = control
            .ctx
            .b
            .build_call(
                &tension_graph_func,
                &[&curve_function_x, &current_tension],
                "curve.y",
                true,
            )
            .left()
            .unwrap()
            .into_float_value();
        let curve_max_value = control
            .ctx
            .b
            .build_load(
                &unsafe {
                    control.ctx.b.build_in_bounds_gep(
                        &start_vals_array_ptr,
                        &[next_loop_index],
                        "curvemax.ptr",
                    )
                },
                "curvemax",
            )
            .into_float_value();

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
        output_num.set_vec(control.ctx.b, output_vec);
        output_num.set_form(
            control.ctx.b,
            control
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
        let new_samples_count = control.ctx.b.build_int_nuw_add(
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
        )
        .into_int_value();
    let state_float =
        control
            .ctx
            .b
            .build_unsigned_int_to_float(state_int, control.ctx.context.f64_type(), "");

    let result_num = NumValue::new(out_val);
    let result_vector = util::splat_vector(control.ctx.b, state_float, "");
    result_num.set_vec(control.ctx.b, result_vector);
    result_num.set_form(
        control.ctx.b,
        control
            .ctx
            .context
            .i8_type()
            .const_int(FormType::None as u64, false),
    );
}

fn state_field_setter(control: &mut ControlContext, in_val: PointerValue) {
    let min_intrinsic = math::min_f64(control.ctx.module);
    let max_intrinsic = math::max_f64(control.ctx.module);

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
        )
        .into_float_value();
    let clamped_left = control
        .ctx
        .b
        .build_call(
            &max_intrinsic,
            &[
                &control.ctx.context.f64_type().const_float(0.),
                &control
                    .ctx
                    .b
                    .build_call(
                        &min_intrinsic,
                        &[
                            &control
                                .ctx
                                .context
                                .f64_type()
                                .const_float(f64::from(u8::max_value()) - 1.),
                            &left_float,
                        ],
                        "",
                        true,
                    )
                    .left()
                    .unwrap()
                    .into_float_value(),
            ],
            "",
            true,
        )
        .left()
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
        )
        .into_int_value();
    let is_paused_float = control.ctx.b.build_unsigned_int_to_float(
        is_paused_int,
        control.ctx.context.f64_type(),
        "paused.float",
    );
    let is_paused_spread = util::splat_vector(&control.ctx.b, is_paused_float, "paused.splat");
    out_num.set_vec(control.ctx.b, is_paused_spread);
    out_num.set_form(
        control.ctx.b,
        control
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
        )
        .into_int_value();
    let current_time_float = control.ctx.b.build_unsigned_int_to_float(
        current_time_int,
        control.ctx.context.f64_type(),
        "time.float",
    );
    let time_spread = util::splat_vector(&control.ctx.b, current_time_float, "time.splat");
    out_num.set_vec(control.ctx.b, time_spread);
    out_num.set_form(
        control.ctx.b,
        control
            .ctx
            .context
            .i8_type()
            .const_int(FormType::Samples as u64, false),
    );
}

fn time_field_setter(control: &mut ControlContext, in_val: PointerValue) {
    let max_intrinsic = math::max_f64(control.ctx.module);

    let in_num = NumValue::new(in_val);
    let in_vec = in_num.get_vec(control.ctx.b);
    let new_time_float = control
        .ctx
        .b
        .build_extract_element(
            &in_vec,
            &control.ctx.context.i64_type().const_int(0, false),
            "time.float",
        )
        .into_float_value();
    let clamped_time = control
        .ctx
        .b
        .build_call(
            &max_intrinsic,
            &[
                &new_time_float,
                &control.ctx.context.f64_type().const_float(0.),
            ],
            "",
            true,
        )
        .left()
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
