use super::{Function, FunctionContext, VarArgs};
use crate::codegen::values::{NumValue, TupleValue};
use crate::codegen::{globals, math, util};
use crate::mir::block;
use inkwell::context::Context;
use inkwell::types::StructType;
use inkwell::values::PointerValue;
use inkwell::IntPredicate;
use std::f64::consts;

const LOOP_COUNT: u64 = 2;

pub struct SvFilterFunction {}
impl Function for SvFilterFunction {
    fn function_type() -> block::Function {
        block::Function::SvFilter
    }

    fn data_type(context: &Context) -> StructType {
        let float_vec = context.f64_type().vec_type(2);
        context.struct_type(
            &[
                &float_vec, // low
                &float_vec, // band
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
        let sin_intrinsic = math::sin_v2f64(func.ctx.module);

        let low_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 0, "low.ptr") };
        let band_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 1, "band.ptr") };

        let input_num = NumValue::new(args[0]);
        let freq_num = NumValue::new(args[1]);
        let q_num = NumValue::new(args[2]);
        let result_tuple = TupleValue::new(result);

        let input_vec = input_num.get_vec(func.ctx.b);
        let freq_vec = freq_num.get_vec(func.ctx.b);
        let q_vec = q_num.get_vec(func.ctx.b);

        let f_val = func.ctx.b.build_float_mul(
            util::get_vec_spread(func.ctx.context, 1.),
            func.ctx
                .b
                .build_call(
                    &sin_intrinsic,
                    &[&func.ctx.b.build_float_div(
                        func.ctx.b.build_float_div(
                            func.ctx.b.build_float_mul(
                                util::get_vec_spread(func.ctx.context, consts::PI),
                                freq_vec,
                                "",
                            ),
                            util::get_vec_spread(func.ctx.context, LOOP_COUNT as f64),
                            "",
                        ),
                        func.ctx
                            .b
                            .build_load(
                                &globals::get_sample_rate(func.ctx.module).as_pointer_value(),
                                "samplerate",
                            )
                            .into_vector_value(),
                        "",
                    )],
                    "",
                    true,
                )
                .left()
                .unwrap()
                .into_vector_value(),
            "f",
        );

        let high_ptr = func
            .ctx
            .allocb
            .build_alloca(&func.ctx.context.f64_type().vec_type(2), "high.ptr");
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
            .append_basic_block(&func.ctx.func, "loopcheck");
        let loop_body_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "loopbody");
        let loop_end_block = func
            .ctx
            .context
            .append_basic_block(&func.ctx.func, "loopend");
        func.ctx.b.build_unconditional_branch(&loop_check_block);

        func.ctx.b.position_at_end(&loop_check_block);
        let current_index = func
            .ctx
            .b
            .build_load(&loop_index_ptr, "index")
            .into_int_value();
        let index_cond = func.ctx.b.build_int_compare(
            IntPredicate::ULT,
            current_index,
            func.ctx.context.i8_type().const_int(LOOP_COUNT, false),
            "indexcond",
        );
        func.ctx
            .b
            .build_conditional_branch(&index_cond, &loop_body_block, &loop_end_block);

        func.ctx.b.position_at_end(&loop_body_block);
        func.ctx.b.build_store(
            &low_ptr,
            &func.ctx.b.build_float_add(
                func.ctx.b.build_load(&low_ptr, "low").into_vector_value(),
                func.ctx.b.build_float_mul(
                    f_val,
                    func.ctx.b.build_load(&band_ptr, "band").into_vector_value(),
                    "",
                ),
                "newlow",
            ),
        );
        func.ctx.b.build_store(
            &high_ptr,
            &func.ctx.b.build_float_sub(
                func.ctx.b.build_float_mul(
                    q_vec,
                    func.ctx.b.build_float_sub(
                        input_vec,
                        func.ctx.b.build_load(&band_ptr, "band").into_vector_value(),
                        "",
                    ),
                    "",
                ),
                func.ctx.b.build_load(&low_ptr, "low").into_vector_value(),
                "newhigh",
            ),
        );
        func.ctx.b.build_store(
            &band_ptr,
            &func.ctx.b.build_float_add(
                func.ctx.b.build_load(&band_ptr, "band").into_vector_value(),
                func.ctx.b.build_float_mul(
                    f_val,
                    func.ctx.b.build_load(&high_ptr, "high").into_vector_value(),
                    "",
                ),
                "newband",
            ),
        );

        let next_index = func.ctx.b.build_int_add(
            current_index,
            func.ctx.context.i8_type().const_int(1, false),
            "nextindex",
        );
        func.ctx.b.build_store(&loop_index_ptr, &next_index);
        func.ctx.b.build_unconditional_branch(&loop_check_block);

        func.ctx.b.position_at_end(&loop_end_block);

        let input_form = input_num.get_form(func.ctx.b);

        let high_result_num = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 0));
        let high_result_vec = func.ctx.b.build_load(&high_ptr, "high").into_vector_value();
        high_result_num.set_vec(func.ctx.b, high_result_vec);
        high_result_num.set_form(func.ctx.b, input_form);

        let low_result_num = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 1));
        let low_result_vec = func.ctx.b.build_load(&low_ptr, "low").into_vector_value();
        low_result_num.set_vec(func.ctx.b, low_result_vec);
        low_result_num.set_form(func.ctx.b, input_form);

        let band_result_num = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 2));
        let band_result_vec = func.ctx.b.build_load(&band_ptr, "band").into_vector_value();
        band_result_num.set_vec(func.ctx.b, band_result_vec);
        band_result_num.set_form(func.ctx.b, input_form);

        let notch_result_num = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 3));
        let notch_result_vec = func
            .ctx
            .b
            .build_float_add(low_result_vec, high_result_vec, "notch");
        notch_result_num.set_vec(func.ctx.b, notch_result_vec);
        notch_result_num.set_form(func.ctx.b, input_form);
    }
}
