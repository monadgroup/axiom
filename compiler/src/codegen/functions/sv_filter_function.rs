use super::{Function, FunctionContext, VarArgs};
use codegen::values::{NumValue, TupleValue};
use codegen::{globals, intrinsics, util};
use inkwell::context::Context;
use inkwell::types::StructType;
use inkwell::values::PointerValue;
use inkwell::IntPredicate;
use mir::block;
use std::f32::consts;

pub struct SvFilterFunction {}
impl Function for SvFilterFunction {
    fn function_type() -> block::Function {
        block::Function::SvFilter
    }

    fn data_type(context: &Context) -> StructType {
        let float_vec = context.f32_type().vec_type(2);
        context.struct_type(
            &[
                &float_vec, // notch
                &float_vec, // low
                &float_vec, // high
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
        let sin_intrinsic = intrinsics::sin_v2f32(func.ctx.module);
        let min_intrinsic = intrinsics::minnum_v2f32(func.ctx.module);
        let pow_intrinsic = intrinsics::pow_v2f32(func.ctx.module);

        let notch_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 0, "notch.ptr") };
        let low_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 1, "low.ptr") };
        let high_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 2, "high.ptr") };
        let band_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 3, "band.ptr") };

        let input_num = NumValue::new(args[0]);
        let freq_num = NumValue::new(args[1]);
        let q_num = NumValue::new(args[2]);
        let result_tuple = TupleValue::new(result);

        let input_vec = input_num.get_vec(func.ctx.b);
        let freq_vec = freq_num.get_vec(func.ctx.b);
        let f_val = func.ctx.b.build_float_mul(
            func.ctx
                .b
                .build_call(
                    &sin_intrinsic,
                    &[&func.ctx.b.build_float_mul(
                        freq_vec,
                        func.ctx.b.build_float_div(
                            util::get_vec_spread(func.ctx.context, consts::PI),
                            func.ctx
                                .b
                                .build_load(
                                    &globals::get_sample_rate(func.ctx.module).as_pointer_value(),
                                    "samplerate",
                                ).into_vector_value(),
                            "",
                        ),
                        "fparam",
                    )],
                    "",
                    false,
                ).left()
                .unwrap()
                .into_vector_value(),
            util::get_vec_spread(func.ctx.context, 2.),
            "f",
        );

        let q_vec = q_num.get_vec(func.ctx.b);
        let q_v =
            func.ctx
                .b
                .build_float_div(util::get_vec_spread(func.ctx.context, 1.), q_vec, "qv");

        // calculate dampening factor
        let damp_val = func.ctx.b.build_float_mul(
            func.ctx.b.build_float_sub(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx
                    .b
                    .build_call(
                        &pow_intrinsic,
                        &[
                            &func.ctx.b.build_float_sub(
                                util::get_vec_spread(func.ctx.context, 1.),
                                func.ctx.b.build_float_div(
                                    util::get_vec_spread(func.ctx.context, 1.),
                                    func.ctx.b.build_float_mul(
                                        q_v,
                                        util::get_vec_spread(func.ctx.context, 2.),
                                        "twoq",
                                    ),
                                    "twoqrecip",
                                ),
                                "damppowbase",
                            ),
                            &util::get_vec_spread(func.ctx.context, 0.25),
                        ],
                        "damppow",
                        false,
                    ).left()
                    .unwrap()
                    .into_vector_value(),
                "inversedamppow",
            ),
            util::get_vec_spread(func.ctx.context, 2.),
            "dampval",
        );

        let max_damp = func
            .ctx
            .b
            .build_call(
                &min_intrinsic,
                &[
                    &util::get_vec_spread(func.ctx.context, 2.),
                    &func.ctx.b.build_float_sub(
                        func.ctx.b.build_float_div(
                            util::get_vec_spread(func.ctx.context, 2.),
                            f_val,
                            "maxdamp.left",
                        ),
                        func.ctx.b.build_float_mul(
                            util::get_vec_spread(func.ctx.context, 0.5),
                            f_val,
                            "maxdamp.right",
                        ),
                        "maxdamp",
                    ),
                ],
                "maxdamp",
                false,
            ).left()
            .unwrap()
            .into_vector_value();
        let damp = func
            .ctx
            .b
            .build_call(&min_intrinsic, &[&damp_val, &max_damp], "damp", false)
            .left()
            .unwrap()
            .into_vector_value();

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
            func.ctx.context.i8_type().const_int(2, false),
            "indexcond",
        );
        func.ctx
            .b
            .build_conditional_branch(&index_cond, &loop_body_block, &loop_end_block);

        func.ctx.b.position_at_end(&loop_body_block);
        func.ctx.b.build_store(
            &notch_ptr,
            &func.ctx.b.build_float_sub(
                input_vec,
                func.ctx.b.build_float_mul(
                    damp,
                    func.ctx.b.build_load(&band_ptr, "band").into_vector_value(),
                    "",
                ),
                "newnotch",
            ),
        );
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
                func.ctx
                    .b
                    .build_load(&notch_ptr, "notch")
                    .into_vector_value(),
                func.ctx.b.build_load(&low_ptr, "low").into_vector_value(),
                "newhigh",
            ),
        );
        func.ctx.b.build_store(
            &band_ptr,
            &func.ctx.b.build_float_add(
                func.ctx.b.build_float_mul(
                    f_val,
                    func.ctx.b.build_load(&high_ptr, "high").into_vector_value(),
                    "",
                ),
                func.ctx.b.build_load(&band_ptr, "band").into_vector_value(),
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
        high_result_num.set_vec(func.ctx.b, &high_result_vec);
        high_result_num.set_form(func.ctx.b, &input_form);

        let low_result_num = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 1));
        let low_result_vec = func.ctx.b.build_load(&low_ptr, "low").into_vector_value();
        low_result_num.set_vec(func.ctx.b, &low_result_vec);
        low_result_num.set_form(func.ctx.b, &input_form);

        let band_result_num = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 2));
        let band_result_vec = func.ctx.b.build_load(&band_ptr, "band").into_vector_value();
        band_result_num.set_vec(func.ctx.b, &band_result_vec);
        band_result_num.set_form(func.ctx.b, &input_form);

        let notch_result_num = NumValue::new(result_tuple.get_item_ptr(func.ctx.b, 3));
        let notch_result_vec = func
            .ctx
            .b
            .build_load(&notch_ptr, "notch")
            .into_vector_value();
        notch_result_num.set_vec(func.ctx.b, &notch_result_vec);
        notch_result_num.set_form(func.ctx.b, &input_form);
    }
}
