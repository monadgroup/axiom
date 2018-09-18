use super::{Function, FunctionContext, VarArgs};
use ast::FormType;
use codegen::values::{ArrayValue, NumValue, ARRAY_CAPACITY};
use codegen::{intrinsics, util};
use inkwell::values::PointerValue;
use inkwell::IntPredicate;
use mir::block;

pub struct IndexedFunction;
impl Function for IndexedFunction {
    fn function_type() -> block::Function {
        block::Function::Indexed
    }

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        _varargs: Option<VarArgs>,
        result: PointerValue,
    ) {
        let min_intrinsic = intrinsics::minnum_f32(func.ctx.module);
        let max_intrinsic = intrinsics::maxnum_f32(func.ctx.module);

        let input_num = NumValue::new(args[0]);
        let result_array = ArrayValue::new(result);

        let input_vec = input_num.get_vec(&mut func.ctx.b);
        let input_count_float = func
            .ctx
            .b
            .build_extract_element(
                &input_vec,
                &func.ctx.context.i64_type().const_int(0, false),
                "input.left",
            )
            .into_float_value();
        let input_count_clamped = func
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
                                &input_count_float,
                                &func.ctx.context.f32_type().const_float(0.),
                            ],
                            "",
                            false,
                        )
                        .left()
                        .unwrap()
                        .into_float_value(),
                    &func
                        .ctx
                        .context
                        .f32_type()
                        .const_float(ARRAY_CAPACITY as f64),
                ],
                "",
                false,
            )
            .left()
            .unwrap()
            .into_float_value();

        let input_count_int = func.ctx.b.build_float_to_unsigned_int(
            input_count_clamped,
            func.ctx.context.i8_type(),
            "input.int",
        );

        // figure out the bitmap for the array
        let shift_amount = func.ctx.b.build_int_nuw_sub(
            func.ctx
                .context
                .i8_type()
                .const_int(ARRAY_CAPACITY as u64, false),
            input_count_int,
            "shiftamount",
        );

        // LLVM defines shifting left by the same number of bits as the input as undefined (so we can't shift a 32-bit number 32 bits left).
        // Since this will happen when the input value is 0, we do the shifting as 64-bit integers and truncate the result to 32 bits.
        let bitmap = func.ctx.b.build_left_shift(
            func.ctx.context.i64_type().const_int(!0u64 as u64, false),
            func.ctx.b.build_int_z_extend_or_bit_cast(
                shift_amount,
                func.ctx.context.i64_type(),
                "",
            ),
            "",
        );
        let truncated_bitmap =
            func.ctx
                .b
                .build_int_truncate(bitmap, func.ctx.context.i32_type(), "");
        result_array.set_bitmap(func.ctx.b, &truncated_bitmap);

        // loop through the indices to build up the output array values
        let loop_index_ptr = func
            .ctx
            .allocb
            .build_alloca(&func.ctx.context.i8_type(), "index.ptr");
        func.ctx.b.build_store(
            &loop_index_ptr,
            &func.ctx.context.i8_type().const_int(0, false),
        );

        let loop_check_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "loop.check");
        let loop_body_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "loop.body");
        let loop_end_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "loop.end");

        func.ctx.b.build_unconditional_branch(&loop_check_block);

        func.ctx.b.position_at_end(&loop_check_block);
        let current_index = func
            .ctx
            .b
            .build_load(&loop_index_ptr, "index")
            .into_int_value();
        let loop_continue = func.ctx.b.build_int_compare(
            IntPredicate::ULT,
            current_index,
            input_count_int,
            "continue",
        );
        func.ctx
            .b
            .build_conditional_branch(&loop_continue, &loop_body_block, &loop_end_block);

        func.ctx.b.position_at_end(&loop_body_block);
        let next_index = func.ctx.b.build_int_nuw_add(
            current_index,
            func.ctx.context.i8_type().const_int(1, false),
            "nextindex",
        );
        func.ctx.b.build_store(&loop_index_ptr, &next_index);

        let index_num = NumValue::new(result_array.get_item_ptr(&mut func.ctx.b, current_index));
        let index_float = func.ctx.b.build_unsigned_int_to_float(
            current_index,
            func.ctx.context.f32_type(),
            "index.float",
        );
        let index_vec = util::splat_vector(func.ctx.b, index_float, "index.vec");
        index_num.set_vec(&mut func.ctx.b, &index_vec);
        index_num.set_form(
            &mut func.ctx.b,
            &func
                .ctx
                .context
                .i8_type()
                .const_int(FormType::None as u64, false),
        );

        func.ctx.b.build_unconditional_branch(&loop_check_block);

        func.ctx.b.position_at_end(&loop_end_block);
    }
}
