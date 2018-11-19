use super::{Function, FunctionContext, VarArgs};
use codegen::values::NumValue;
use codegen::{
    build_context_function, globals, intrinsics, util, BuilderContext, TargetProperties,
};
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::StructType;
use inkwell::values::{FunctionValue, PointerValue};
use inkwell::{AddressSpace, IntPredicate};
use mir::block;

pub struct DelayFunction {}
impl DelayFunction {
    fn get_channel_update_func(module: &Module) -> FunctionValue {
        util::get_or_create_func(module, "maxim.util.delay.channelUpdate", true, &|| {
            let context = &module.get_context();
            (
                Linkage::PrivateLinkage,
                context.f64_type().fn_type(
                    &[
                        &context.i64_type().ptr_type(AddressSpace::Generic), // current position pointer
                        &context.i64_type().ptr_type(AddressSpace::Generic), // current size pointer
                        &context.i64_type(),                                 // delay sample count
                        &context.i64_type(),                                 // reserve sample count
                        &context
                            .f64_type()
                            .ptr_type(AddressSpace::Generic)
                            .ptr_type(AddressSpace::Generic), // samples pointer pointer
                        &context.f64_type(),                                 // input value
                    ],
                    false,
                ),
            )
        })
    }

    /// Builds a function that is equivalent to the following C++:
    /// ```cpp
    /// float channelUpdate(uint64_t *currentPos, uint64_t *currentSize, uint64_t delaySamples, uint64_t reserveSamples, float **buffer, float input) {
    ///     float resultVal;
    ///
    ///     if (*currentSize) {
    ///         uint64_t loadedCurrentPos = *currentPos;
    ///         *currentPos = (loadedCurrentPos + 1) % *currentSize;
    ///
    ///         if (delaySamples == 0) {
    ///             resultVal = input;
    ///         } else {
    ///             auto readPosition = (loadedCurrentPos + *currentSize - delaySamples) % *currentSize;
    ///             resultVal = (*buffer)[readPosition];
    ///         }
    ///
    ///         (*buffer)[loadedCurrentPos] = input;
    ///     } else {
    ///         resultVal = input;
    ///     }
    ///
    ///     auto bufferSize = calculateNextPowerOfTwo(reserveSamples);
    ///     if (bufferSize != *currentSize) {
    ///         *buffer = realloc(*buffer, bufferSize * sizeof(float));
    ///         if (bufferSize == 0) *buffer = nullptr;
    ///         else memset(*buffer + *currentSize, 0, (bufferSize - *currentSize) * sizeof(float));
    ///         *currentSize = bufferSize;
    ///     }
    ///
    ///     return resultVal;
    /// }
    /// ```
    fn build_channel_update_func(module: &Module, target: &TargetProperties) {
        let func = DelayFunction::get_channel_update_func(module);
        build_context_function(module, func, target, &|ctx: BuilderContext| {
            let target_data = target.machine.get_data();
            let next_power_intrinsic = intrinsics::next_power_i64(ctx.module);
            let realloc_intrinsic = intrinsics::realloc(ctx.module, &target_data);
            let memset_intrinsic = intrinsics::memset(ctx.module, &target_data);

            let current_pos_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();
            let current_size_ptr = ctx.func.get_nth_param(1).unwrap().into_pointer_value();
            let delay_samples = ctx.func.get_nth_param(2).unwrap().into_int_value();
            let reserve_samples = ctx.func.get_nth_param(3).unwrap().into_int_value();
            let buffer_ptr_ptr = ctx.func.get_nth_param(4).unwrap().into_pointer_value();
            let input_num = ctx.func.get_nth_param(5).unwrap().into_float_value();

            let has_buffer_true_block = ctx.context.append_basic_block(&ctx.func, "hasbuffer.true");
            let has_samples_true_block =
                ctx.context.append_basic_block(&ctx.func, "hassamples.true");
            let has_samples_false_block = ctx
                .context
                .append_basic_block(&ctx.func, "hassamples.false");
            let has_samples_continue_block = ctx
                .context
                .append_basic_block(&ctx.func, "hassamples.continue");
            let has_buffer_false_block =
                ctx.context.append_basic_block(&ctx.func, "hasbuffer.false");
            let has_buffer_continue_block = ctx
                .context
                .append_basic_block(&ctx.func, "hasbuffer.continue");
            let needs_realloc_true_block = ctx
                .context
                .append_basic_block(&ctx.func, "needsrealloc.true");
            let size_zero_true_block = ctx.context.append_basic_block(&ctx.func, "sizezero.true");
            let size_zero_false_block = ctx.context.append_basic_block(&ctx.func, "sizezero.false");
            let size_increase_true_block = ctx
                .context
                .append_basic_block(&ctx.func, "sizeincrease.true");
            let size_increase_continue_block = ctx
                .context
                .append_basic_block(&ctx.func, "sizeincrease.continue");
            let size_zero_continue_block = ctx
                .context
                .append_basic_block(&ctx.func, "sizezero.continue");
            let needs_realloc_continue_block = ctx
                .context
                .append_basic_block(&ctx.func, "needsrealloc.continue");

            let result_ptr = ctx
                .allocb
                .build_alloca(&ctx.context.f64_type(), "resultval");
            let buffer_ptr = ctx
                .b
                .build_load(&buffer_ptr_ptr, "bufferptr")
                .into_pointer_value();

            // if (*currentSize) {
            let current_size = ctx
                .b
                .build_load(&current_size_ptr, "currentsize")
                .into_int_value();
            let has_buffer = ctx.b.build_int_compare(
                IntPredicate::UGT,
                current_size,
                ctx.context.i64_type().const_int(0, false),
                "hasbuffer",
            );
            ctx.b.build_conditional_branch(
                &has_buffer,
                &has_buffer_true_block,
                &has_buffer_false_block,
            );

            ctx.b.position_at_end(&has_buffer_true_block);

            // uint64_t loadedCurrentPos = *currentPos;
            let current_pos = ctx
                .b
                .build_load(&current_pos_ptr, "currentpos")
                .into_int_value();

            // *currentPos = (loadedCurrentPos + 1) % *currentSize;
            let new_pos = ctx.b.build_int_unsigned_rem(
                ctx.b.build_int_nuw_add(
                    current_pos,
                    ctx.context.i64_type().const_int(1, false),
                    "newpos.unbounded",
                ),
                current_size,
                "newpos",
            );
            ctx.b.build_store(&current_pos_ptr, &new_pos);

            // if (delaySamples == 0) {
            let has_samples = ctx.b.build_int_compare(
                IntPredicate::NE,
                delay_samples,
                ctx.context.i64_type().const_int(0, false),
                "hassamples",
            );
            ctx.b.build_conditional_branch(
                &has_samples,
                &has_samples_true_block,
                &has_samples_false_block,
            );

            ctx.b.position_at_end(&has_samples_true_block);

            // auto readPosition = (loadedCurrentPos + *currentSize - delaySamples) % delaySamples;
            let read_position = ctx.b.build_int_unsigned_rem(
                ctx.b.build_int_nuw_sub(
                    ctx.b.build_int_nuw_add(current_pos, current_size, ""),
                    delay_samples,
                    "",
                ),
                current_size,
                "readposition",
            );

            // resultVal = (*buffer)[readPosition];
            let result_val = ctx
                .b
                .build_load(
                    &unsafe {
                        ctx.b
                            .build_in_bounds_gep(&buffer_ptr, &[read_position], "result.ptr")
                    },
                    "result",
                ).into_float_value();
            ctx.b.build_store(&result_ptr, &result_val);
            ctx.b
                .build_unconditional_branch(&has_samples_continue_block);

            // } else {
            ctx.b.position_at_end(&has_samples_false_block);

            // resultVal = input;
            ctx.b.build_store(&result_ptr, &input_num);
            ctx.b
                .build_unconditional_branch(&has_samples_continue_block);

            // }
            ctx.b.position_at_end(&has_samples_continue_block);

            // (*buffer)[loadedCurrentPos] = input;
            ctx.b.build_store(
                &unsafe {
                    ctx.b
                        .build_in_bounds_gep(&buffer_ptr, &[current_pos], "write.ptr")
                },
                &input_num,
            );
            ctx.b.build_unconditional_branch(&has_buffer_continue_block);

            ctx.b.position_at_end(&has_buffer_false_block);

            // resultVal = input;
            ctx.b.build_store(&result_ptr, &input_num);
            ctx.b.build_unconditional_branch(&has_buffer_continue_block);

            ctx.b.position_at_end(&has_buffer_continue_block);

            // auto bufferSize = calculateNextPowerOfTwo(reserveSamples);
            let new_buffer_size = ctx
                .b
                .build_call(
                    &next_power_intrinsic,
                    &[&reserve_samples],
                    "newbuffersize",
                    false,
                ).left()
                .unwrap()
                .into_int_value();

            // if (bufferSize != *currentSize) {
            let needs_realloc = ctx.b.build_int_compare(
                IntPredicate::NE,
                new_buffer_size,
                current_size,
                "needsrealloc",
            );
            ctx.b.build_conditional_branch(
                &needs_realloc,
                &needs_realloc_true_block,
                &needs_realloc_continue_block,
            );

            ctx.b.position_at_end(&needs_realloc_true_block);

            // *buffer = realloc(*buffer, bufferSize * sizeof(float));
            let size_type = target_data.int_ptr_type_in_context(ctx.context);
            let float_size = ctx
                .context
                .f64_type()
                .size_of()
                .const_cast(&size_type, false);
            let realloc_size = ctx.b.build_int_nuw_mul(
                ctx.b.build_int_cast(new_buffer_size, size_type, ""),
                float_size,
                "",
            );
            let realloc_ptr = ctx
                .b
                .build_call(
                    &realloc_intrinsic,
                    &[
                        &ctx.b.build_pointer_cast(
                            buffer_ptr,
                            ctx.context.i8_type().ptr_type(AddressSpace::Generic),
                            "",
                        ),
                        &realloc_size,
                    ],
                    "",
                    false,
                ).left()
                .unwrap()
                .into_pointer_value();

            // if (bufferSize == 0) *buffer = nullptr;
            let size_is_zero = ctx.b.build_int_compare(
                IntPredicate::EQ,
                realloc_size,
                size_type.const_int(0, false),
                "sizezero",
            );
            ctx.b.build_conditional_branch(
                &size_is_zero,
                &size_zero_true_block,
                &size_zero_false_block,
            );

            let buffer_ptr_type = ctx.context.f64_type().ptr_type(AddressSpace::Generic);

            ctx.b.position_at_end(&size_zero_true_block);
            ctx.b
                .build_store(&buffer_ptr_ptr, &buffer_ptr_type.const_null());
            ctx.b.build_unconditional_branch(&size_zero_continue_block);

            // else if (bufferSize > *currentSize) memset(*buffer + *currentSize, 0, (bufferSize - *currentSize) * sizeof(float));
            ctx.b.position_at_end(&size_zero_false_block);
            let size_increase = ctx.b.build_int_compare(
                IntPredicate::UGT,
                new_buffer_size,
                current_size,
                "sizeincrease",
            );
            ctx.b.build_conditional_branch(
                &size_increase,
                &size_increase_true_block,
                &size_increase_continue_block,
            );

            ctx.b.position_at_end(&size_increase_true_block);
            let current_size_bytes = ctx.b.build_int_nuw_mul(
                ctx.b.build_int_cast(current_size, size_type, ""),
                float_size,
                "currentsizebytes",
            );
            ctx.b.build_call(
                &memset_intrinsic,
                &[
                    &unsafe {
                        ctx.b
                            .build_in_bounds_gep(&realloc_ptr, &[current_size_bytes], "offsetptr")
                    },
                    &ctx.context.i8_type().const_int(0, false),
                    &ctx.b
                        .build_int_nuw_sub(realloc_size, current_size_bytes, ""),
                    &ctx.context.i32_type().const_int(0, false),
                    &ctx.context.bool_type().const_int(0, false),
                ],
                "",
                false,
            );
            ctx.b
                .build_unconditional_branch(&size_increase_continue_block);

            ctx.b.position_at_end(&size_increase_continue_block);
            ctx.b.build_store(
                &buffer_ptr_ptr,
                &ctx.b
                    .build_pointer_cast(realloc_ptr, buffer_ptr_type, "newbufferptr"),
            );
            ctx.b.build_unconditional_branch(&size_zero_continue_block);

            ctx.b.position_at_end(&size_zero_continue_block);

            // *currentSize = bufferSize;
            ctx.b.build_store(&current_size_ptr, &new_buffer_size);
            ctx.b
                .build_unconditional_branch(&needs_realloc_continue_block);

            ctx.b.position_at_end(&needs_realloc_continue_block);
            ctx.b.build_return(Some(&ctx.b.build_load(&result_ptr, "")));
        });
    }
}

impl Function for DelayFunction {
    fn function_type() -> block::Function {
        block::Function::Delay
    }

    fn data_type(context: &Context) -> StructType {
        let size_type = context.i64_type();
        let channel_type = context.f64_type();

        context.struct_type(
            &[
                &size_type,                                    // left position
                &size_type,                                    // right position
                &size_type,                                    // left buffer length
                &size_type,                                    // right buffer length
                &channel_type.ptr_type(AddressSpace::Generic), // left buffer
                &channel_type.ptr_type(AddressSpace::Generic), // right buffer
            ],
            false,
        )
    }

    fn gen_real_args(ctx: &mut BuilderContext, mut args: Vec<PointerValue>) -> Vec<PointerValue> {
        if args.len() < 3 {
            let mut delay_constant = NumValue::new_undef(ctx.context, ctx.allocb);
            delay_constant.store(ctx.b, &NumValue::get_const(ctx.context, 1., 1., 0));
            args.insert(1, delay_constant.val);
        }
        args
    }

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        _varargs: Option<VarArgs>,
        result: PointerValue,
    ) {
        let min_intrinsic = intrinsics::minnum_v2f64(func.ctx.module);
        let max_intrinsic = intrinsics::maxnum_v2f64(func.ctx.module);

        DelayFunction::build_channel_update_func(func.ctx.module, func.ctx.target);
        let channel_update_func = DelayFunction::get_channel_update_func(func.ctx.module);

        let left_pos_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 0, "leftpos.ptr")
        };
        let right_pos_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 1, "rightpos.ptr")
        };
        let left_buffer_length_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 2, "leftbuflength.ptr")
        };
        let right_buffer_length_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 3, "rightbuflength")
        };
        let left_buffer_ptr_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 4, "leftbuffer.ptr")
        };
        let right_buffer_ptr_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 5, "rightbuffer.ptr")
        };

        let input_num = NumValue::new(args[0]);
        let delay_num = NumValue::new(args[1]);
        let reserve_num = NumValue::new(args[2]);
        let result_num = NumValue::new(result);

        let sample_rate = func
            .ctx
            .b
            .build_load(
                &globals::get_sample_rate(func.ctx.module).as_pointer_value(),
                "samplerate",
            ).into_vector_value();

        // determine reserve samples
        let reserve_vec = reserve_num.get_vec(func.ctx.b);
        let reserve_samples_float = func
            .ctx
            .b
            .build_call(
                &max_intrinsic,
                &[
                    &func.ctx.b.build_float_mul(
                        reserve_vec,
                        sample_rate,
                        "reservesamples.unclamped",
                    ),
                    &util::get_vec_spread(func.ctx.context, 0.),
                ],
                "reservesamples.clamped",
                false,
            ).left()
            .unwrap()
            .into_vector_value();
        let reserve_samples = func.ctx.b.build_float_to_unsigned_int(
            reserve_samples_float,
            func.ctx.context.i64_type().vec_type(2),
            "reservesamples.int",
        );

        // saturate delayVal so it can't be out of bounds
        let delay_vec = delay_num.get_vec(func.ctx.b);
        let delay_val_clamped = func
            .ctx
            .b
            .build_call(
                &max_intrinsic,
                &[
                    &func
                        .ctx
                        .b
                        .build_call(
                            &min_intrinsic,
                            &[&delay_vec, &util::get_vec_spread(func.ctx.context, 1.)],
                            "delayval.clamped",
                            false,
                        ).left()
                        .unwrap()
                        .into_vector_value(),
                    &util::get_vec_spread(func.ctx.context, 0.),
                ],
                "delayval.clamped",
                false,
            ).left()
            .unwrap()
            .into_vector_value();
        let delay_samples_float = func.ctx.b.build_float_mul(
            delay_val_clamped,
            reserve_samples_float,
            "delaysamples.float",
        );
        let delay_samples = func.ctx.b.build_float_to_unsigned_int(
            delay_samples_float,
            func.ctx.context.i64_type().vec_type(2),
            "delaysamples.int",
        );

        // update the buffer
        let input_vec = input_num.get_vec(func.ctx.b);
        let left_element = func.ctx.context.i32_type().const_int(0, false);
        let left_result = func
            .ctx
            .b
            .build_call(
                &channel_update_func,
                &[
                    &left_pos_ptr,
                    &left_buffer_length_ptr,
                    &func
                        .ctx
                        .b
                        .build_extract_element(&delay_samples, &left_element, ""),
                    &func
                        .ctx
                        .b
                        .build_extract_element(&reserve_samples, &left_element, ""),
                    &left_buffer_ptr_ptr,
                    &func
                        .ctx
                        .b
                        .build_extract_element(&input_vec, &left_element, ""),
                ],
                "result.left",
                true,
            ).left()
            .unwrap()
            .into_float_value();

        let right_element = func.ctx.context.i32_type().const_int(1, false);
        let right_result = func
            .ctx
            .b
            .build_call(
                &channel_update_func,
                &[
                    &right_pos_ptr,
                    &right_buffer_length_ptr,
                    &func
                        .ctx
                        .b
                        .build_extract_element(&delay_samples, &right_element, ""),
                    &func
                        .ctx
                        .b
                        .build_extract_element(&reserve_samples, &right_element, ""),
                    &right_buffer_ptr_ptr,
                    &func
                        .ctx
                        .b
                        .build_extract_element(&input_vec, &right_element, ""),
                ],
                "result.right",
                true,
            ).left()
            .unwrap()
            .into_float_value();

        let result_vec = func
            .ctx
            .b
            .build_insert_element(
                &func
                    .ctx
                    .b
                    .build_insert_element(
                        &func.ctx.context.f64_type().vec_type(2).get_undef(),
                        &left_result,
                        &left_element,
                        "",
                    ).into_vector_value(),
                &right_result,
                &right_element,
                "",
            ).into_vector_value();
        result_num.set_vec(func.ctx.b, &result_vec);

        let input_form = input_num.get_form(func.ctx.b);
        result_num.set_form(func.ctx.b, &input_form);
    }

    fn gen_destruct(func: &mut FunctionContext) {
        let left_buffer_ptr = func
            .ctx
            .b
            .build_load(
                &unsafe {
                    func.ctx
                        .b
                        .build_struct_gep(&func.data_ptr, 4, "leftbuffer.ptr")
                },
                "leftbuffer",
            ).into_pointer_value();
        let right_buffer_ptr = func
            .ctx
            .b
            .build_load(
                &unsafe {
                    func.ctx
                        .b
                        .build_struct_gep(&func.data_ptr, 5, "rightbuffer.ptr")
                },
                "rightbuffer",
            ).into_pointer_value();
        func.ctx.b.build_free(&left_buffer_ptr);
        func.ctx.b.build_free(&right_buffer_ptr);
    }
}
