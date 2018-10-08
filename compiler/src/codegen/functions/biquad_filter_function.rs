use super::{Function, FunctionContext, VarArgs};
use codegen::values::NumValue;
use codegen::{
    build_context_function, globals, intrinsics, util, BuilderContext, TargetProperties,
};
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicType, StructType};
use inkwell::values::{FunctionValue, PointerValue, VectorValue};
use inkwell::AddressSpace;
use inkwell::FloatPredicate;
use mir::block;
use std::f32::consts;

fn get_internal_biquad_func(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.util.biquad.biquadFilter", true, &|| {
        let context = &module.get_context();
        let float_vec = context.f32_type().vec_type(2);
        let func = float_vec.fn_type(
            &[
                &float_vec,                                 // input
                &float_vec,                                 // a0
                &float_vec,                                 // a1
                &float_vec,                                 // a2
                &float_vec,                                 // b1
                &float_vec,                                 // b2
                &float_vec.ptr_type(AddressSpace::Generic), // z1
                &float_vec.ptr_type(AddressSpace::Generic), // z2
            ],
            false,
        );

        (Linkage::PrivateLinkage, func)
    })
}

pub fn build_internal_biquad_func(module: &Module, target: &TargetProperties) {
    let func = get_internal_biquad_func(module);
    build_context_function(module, func, target, &|ctx: BuilderContext| {
        let input_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();
        let a0_vec = ctx.func.get_nth_param(1).unwrap().into_vector_value();
        let a1_vec = ctx.func.get_nth_param(2).unwrap().into_vector_value();
        let a2_vec = ctx.func.get_nth_param(3).unwrap().into_vector_value();
        let b1_vec = ctx.func.get_nth_param(4).unwrap().into_vector_value();
        let b2_vec = ctx.func.get_nth_param(5).unwrap().into_vector_value();
        let z1_vec_ptr = ctx.func.get_nth_param(6).unwrap().into_pointer_value();
        let z2_vec_ptr = ctx.func.get_nth_param(7).unwrap().into_pointer_value();

        let out_vec = ctx.b.build_float_add(
            ctx.b.build_float_mul(input_vec, a0_vec, ""),
            ctx.b.build_load(&z1_vec_ptr, "z1").into_vector_value(),
            "",
        );

        let new_z1 = ctx.b.build_float_sub(
            ctx.b.build_float_add(
                ctx.b.build_float_mul(input_vec, a1_vec, ""),
                ctx.b.build_load(&z2_vec_ptr, "z2").into_vector_value(),
                "",
            ),
            ctx.b.build_float_mul(b1_vec, out_vec, ""),
            "newz1",
        );
        ctx.b.build_store(&z1_vec_ptr, &new_z1);

        let new_z2 = ctx.b.build_float_sub(
            ctx.b.build_float_mul(input_vec, a2_vec, ""),
            ctx.b.build_float_mul(b2_vec, out_vec, ""),
            "",
        );
        ctx.b.build_store(&z2_vec_ptr, &new_z2);

        ctx.b.build_return(Some(&out_vec));
    });
}

fn biquad_data_type(context: &Context, has_gain: bool) -> StructType {
    let vec_type = context.f32_type().vec_type(2);
    let mut field_types: Vec<&BasicType> = vec![
        &vec_type, // a0
        &vec_type, // a1
        &vec_type, // a2
        &vec_type, // b1
        &vec_type, // b2
        &vec_type, // z1
        &vec_type, // z2
        &vec_type, // cached frequency
        &vec_type, // cached Q
    ];
    if has_gain {
        field_types.push(&vec_type);
    }

    context.struct_type(&field_types, false)
}

type GenerateCoefficientsFn = Fn(
    &mut FunctionContext,
    VectorValue,
    VectorValue,
    VectorValue,
    Option<VectorValue>,
    PointerValue,
    PointerValue,
    PointerValue,
    PointerValue,
    PointerValue,
);

fn gen_biquad_call(
    func: &mut FunctionContext,
    args: &[PointerValue],
    result: PointerValue,
    has_gain: bool,
    generate_coefficients: &GenerateCoefficientsFn,
) {
    let max_intrinsic = intrinsics::maxnum_v2f32(func.ctx.module);
    let tan_intrinsic = util::get_or_create_func(func.ctx.module, "tanf", false, &|| {
        (
            Linkage::ExternalLinkage,
            func.ctx
                .context
                .f32_type()
                .fn_type(&[&func.ctx.context.f32_type()], false),
        )
    });
    let internal_biquad_func = get_internal_biquad_func(func.ctx.module);

    let a0_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 0, "a0.ptr") };
    let a1_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 1, "a1.ptr") };
    let a2_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 2, "a2.ptr") };
    let b1_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 3, "b1.ptr") };
    let b2_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 4, "b2.ptr") };
    let z1_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 5, "z1.ptr") };
    let z2_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 6, "z2.ptr") };
    let cached_freq_ptr = unsafe {
        func.ctx
            .b
            .build_struct_gep(&func.data_ptr, 7, "cachedfreq.ptr")
    };
    let cached_q_ptr = unsafe {
        func.ctx
            .b
            .build_struct_gep(&func.data_ptr, 8, "cachedq.ptr")
    };
    let cached_gain_ptr = if has_gain {
        Some(unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 9, "cachedgain.ptr")
        })
    } else {
        None
    };

    let input_num = NumValue::new(args[0]);
    let freq_num = NumValue::new(args[1]);
    let q_num = NumValue::new(args[2]);
    let result_num = NumValue::new(result);

    let gain_vec = if has_gain {
        Some(NumValue::new(args[3]).get_vec(func.ctx.b))
    } else {
        None
    };

    let freq_vec = freq_num.get_vec(func.ctx.b);
    let q_vec = q_num.get_vec(func.ctx.b);

    let cached_freq = func
        .ctx
        .b
        .build_load(&cached_freq_ptr, "cachedfreq")
        .into_vector_value();
    let freq_changed =
        func.ctx
            .b
            .build_float_compare(FloatPredicate::ONE, freq_vec, cached_freq, "freqchanged");
    let cached_q = func
        .ctx
        .b
        .build_load(&cached_q_ptr, "cachedq")
        .into_vector_value();
    let q_changed =
        func.ctx
            .b
            .build_float_compare(FloatPredicate::ONE, q_vec, cached_q, "qchanged");
    let needs_regen_vec = func.ctx.b.build_or(freq_changed, q_changed, "needsregen");
    let needs_regen_vec = if let Some(cached_gain_ptr) = cached_gain_ptr {
        let cached_gain = func
            .ctx
            .b
            .build_load(&cached_gain_ptr, "cachedgain")
            .into_vector_value();
        let gain_changed = func.ctx.b.build_float_compare(
            FloatPredicate::ONE,
            gain_vec.unwrap(),
            cached_gain,
            "gainchanged",
        );
        func.ctx
            .b
            .build_or(needs_regen_vec, gain_changed, "needsregen")
    } else {
        needs_regen_vec
    };

    let left_element = func.ctx.context.i32_type().const_int(0, false);
    let right_element = func.ctx.context.i32_type().const_int(1, false);
    let needs_regen = func.ctx.b.build_or(
        func.ctx
            .b
            .build_extract_element(&needs_regen_vec, &left_element, "")
            .into_int_value(),
        func.ctx
            .b
            .build_extract_element(&needs_regen_vec, &right_element, "")
            .into_int_value(),
        "",
    );

    let needs_regen_true_block = func
        .ctx
        .context
        .append_basic_block(&func.ctx.func, "needsregen.true");
    let needs_regen_continue_block = func
        .ctx
        .context
        .append_basic_block(&func.ctx.func, "needsregen.continue");
    func.ctx.b.build_conditional_branch(
        &needs_regen,
        &needs_regen_true_block,
        &needs_regen_continue_block,
    );

    func.ctx.b.position_at_end(&needs_regen_true_block);
    func.ctx.b.build_store(&cached_freq_ptr, &freq_vec);
    func.ctx.b.build_store(&cached_q_ptr, &q_vec);
    if let Some(cached_gain_ptr) = cached_gain_ptr {
        func.ctx.b.build_store(&cached_gain_ptr, &gain_vec.unwrap());
    }

    // ensure Q is 0.5 or above to avoid dividing by zero later on
    let q_vec = func
        .ctx
        .b
        .build_call(
            &max_intrinsic,
            &[&q_vec, &util::get_vec_spread(func.ctx.context, 0.5)],
            "",
            false,
        ).left()
        .unwrap()
        .into_vector_value();

    // calculate K value = tan(PI * freq / sampleRate)
    let k_param = func.ctx.b.build_float_div(
        func.ctx.b.build_float_mul(
            util::get_vec_spread(func.ctx.context, consts::PI),
            freq_vec,
            "",
        ),
        func.ctx
            .b
            .build_load(
                &globals::get_sample_rate(func.ctx.module).as_pointer_value(),
                "",
            ).into_vector_value(),
        "",
    );
    let left_k = func
        .ctx
        .b
        .build_call(
            &tan_intrinsic,
            &[&func
                .ctx
                .b
                .build_extract_element(&k_param, &left_element, "")],
            "leftk",
            false,
        ).left()
        .unwrap()
        .into_float_value();
    let right_k = func
        .ctx
        .b
        .build_call(
            &tan_intrinsic,
            &[&func
                .ctx
                .b
                .build_extract_element(&k_param, &right_element, "")],
            "rightk",
            false,
        ).left()
        .unwrap()
        .into_float_value();
    let k_value = func
        .ctx
        .b
        .build_insert_element(
            &func
                .ctx
                .b
                .build_insert_element(
                    &func.ctx.context.f32_type().vec_type(2).get_undef(),
                    &left_k,
                    &left_element,
                    "",
                ).into_vector_value(),
            &right_k,
            &right_element,
            "k",
        ).into_vector_value();
    let k_squared = func.ctx.b.build_float_mul(k_value, k_value, "ksquared");

    generate_coefficients(
        func, q_vec, k_value, k_squared, gain_vec, a0_ptr, a1_ptr, a2_ptr, b1_ptr, b2_ptr,
    );
    func.ctx
        .b
        .build_unconditional_branch(&needs_regen_continue_block);

    func.ctx.b.position_at_end(&needs_regen_continue_block);
    let input_vec = input_num.get_vec(func.ctx.b);
    let input_form = input_num.get_form(func.ctx.b);
    let result_vec = func
        .ctx
        .b
        .build_call(
            &internal_biquad_func,
            &[
                &input_vec,
                &func.ctx.b.build_load(&a0_ptr, "a0").into_vector_value(),
                &func.ctx.b.build_load(&a1_ptr, "a1").into_vector_value(),
                &func.ctx.b.build_load(&a2_ptr, "a2").into_vector_value(),
                &func.ctx.b.build_load(&b1_ptr, "b1").into_vector_value(),
                &func.ctx.b.build_load(&b2_ptr, "b2").into_vector_value(),
                &z1_ptr,
                &z2_ptr,
            ],
            "resultvec",
            false,
        ).left()
        .unwrap()
        .into_vector_value();
    result_num.set_vec(func.ctx.b, &result_vec);
    result_num.set_form(func.ctx.b, &input_form);
}

macro_rules! define_biquad_func (
    ($func_name:ident: $func_type:expr => $has_gain:expr, $callback:expr) => (
        pub struct $func_name {}
        impl Function for $func_name {
            fn function_type() -> block::Function { $func_type }
            fn data_type(context: &Context) -> StructType {
                biquad_data_type(context, $has_gain)
            }
            fn gen_call(func: &mut FunctionContext, args: &[PointerValue], _varargs: Option<VarArgs>, result: PointerValue) {
                gen_biquad_call(func, args, result, $has_gain, &$callback)
            }
        }
    )
);

fn low_filter_generate_coefficients(
    func: &mut FunctionContext,
    q_vec: VectorValue,
    k_value: VectorValue,
    k_squared: VectorValue,
    _gain_vec: Option<VectorValue>,
    a0_ptr: PointerValue,
    a1_ptr: PointerValue,
    a2_ptr: PointerValue,
    b1_ptr: PointerValue,
    b2_ptr: PointerValue,
) {
    // norm = 1 / (1 + K / q + K * K)
    let norm = func.ctx.b.build_float_div(
        util::get_vec_spread(func.ctx.context, 1.),
        func.ctx.b.build_float_add(
            func.ctx.b.build_float_add(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx.b.build_float_div(k_value, q_vec, ""),
                "",
            ),
            k_squared,
            "",
        ),
        "",
    );

    // a0 = K * K * norm
    let a0 = func.ctx.b.build_float_mul(k_squared, norm, "");
    func.ctx.b.build_store(&a0_ptr, &a0);

    // a1 = 2 * a0
    let a1 = func
        .ctx
        .b
        .build_float_mul(util::get_vec_spread(func.ctx.context, 2.), a0, "");
    func.ctx.b.build_store(&a1_ptr, &a1);

    // a2 = a0
    let a2 = a0;
    func.ctx.b.build_store(&a2_ptr, &a2);

    // b1 = 2 * (K * K - 1) * norm
    let b1 = func.ctx.b.build_float_mul(
        func.ctx.b.build_float_mul(
            util::get_vec_spread(func.ctx.context, 2.),
            func.ctx
                .b
                .build_float_sub(k_squared, util::get_vec_spread(func.ctx.context, 1.), ""),
            "",
        ),
        norm,
        "",
    );
    func.ctx.b.build_store(&b1_ptr, &b1);

    // b2 = (1 - K / q + K * K) * norm
    let b2 = func.ctx.b.build_float_mul(
        func.ctx.b.build_float_add(
            func.ctx.b.build_float_sub(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx.b.build_float_div(k_value, q_vec, ""),
                "",
            ),
            k_squared,
            "",
        ),
        norm,
        "",
    );
    func.ctx.b.build_store(&b2_ptr, &b2);
}
define_biquad_func!(LowBqFilterFunction: block::Function::LowBqFilter => false, low_filter_generate_coefficients);

fn high_filter_generate_coefficients(
    func: &mut FunctionContext,
    q_vec: VectorValue,
    k_value: VectorValue,
    k_squared: VectorValue,
    _gain_vec: Option<VectorValue>,
    a0_ptr: PointerValue,
    a1_ptr: PointerValue,
    a2_ptr: PointerValue,
    b1_ptr: PointerValue,
    b2_ptr: PointerValue,
) {
    // norm = 1 / (1 + K / q + K * K)
    let norm = func.ctx.b.build_float_div(
        util::get_vec_spread(func.ctx.context, 1.),
        func.ctx.b.build_float_add(
            func.ctx.b.build_float_add(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx.b.build_float_div(k_value, q_vec, ""),
                "",
            ),
            k_squared,
            "",
        ),
        "",
    );

    // a0 = norm
    let a0 = norm;
    func.ctx.b.build_store(&a0_ptr, &a0);

    // a1 = -2 * a0
    let a1 = func
        .ctx
        .b
        .build_float_mul(util::get_vec_spread(func.ctx.context, -2.), a0, "");
    func.ctx.b.build_store(&a1_ptr, &a1);

    // a2 = a0
    let a2 = a0;
    func.ctx.b.build_store(&a2_ptr, &a2);

    // b1 = 2 * (K * K - 1) * norm
    let b1 = func.ctx.b.build_float_mul(
        func.ctx.b.build_float_mul(
            util::get_vec_spread(func.ctx.context, 2.),
            func.ctx
                .b
                .build_float_sub(k_squared, util::get_vec_spread(func.ctx.context, 1.), ""),
            "",
        ),
        norm,
        "",
    );
    func.ctx.b.build_store(&b1_ptr, &b1);

    // b2 = (1 - K / q + K * K) * norm
    let b2 = func.ctx.b.build_float_mul(
        func.ctx.b.build_float_add(
            func.ctx.b.build_float_sub(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx.b.build_float_div(k_value, q_vec, ""),
                "",
            ),
            k_squared,
            "",
        ),
        norm,
        "",
    );
    func.ctx.b.build_store(&b2_ptr, &b2);
}
define_biquad_func!(HighBqFilterFunction: block::Function::HighBqFilter => false, high_filter_generate_coefficients);

fn peak_filter_generate_coefficients(
    func: &mut FunctionContext,
    q_vec: VectorValue,
    k_value: VectorValue,
    k_squared: VectorValue,
    gain_vec: Option<VectorValue>,
    a0_ptr: PointerValue,
    a1_ptr: PointerValue,
    a2_ptr: PointerValue,
    b1_ptr: PointerValue,
    b2_ptr: PointerValue,
) {
    let pow_intrinsic = intrinsics::pow_v2f32(func.ctx.module);
    let abs_intrinsic = intrinsics::fabs_v2f32(func.ctx.module);

    let gain_vec = gain_vec.unwrap();
    let v = func
        .ctx
        .b
        .build_call(
            &pow_intrinsic,
            &[
                &util::get_vec_spread(func.ctx.context, 10.),
                &func.ctx.b.build_float_div(
                    func.ctx
                        .b
                        .build_call(&abs_intrinsic, &[&gain_vec], "", false)
                        .left()
                        .unwrap()
                        .into_vector_value(),
                    util::get_vec_spread(func.ctx.context, 20.),
                    "",
                ),
            ],
            "",
            false,
        ).left()
        .unwrap()
        .into_vector_value();

    let is_gain_positive_vec = func.ctx.b.build_float_compare(
        FloatPredicate::OGE,
        gain_vec,
        util::get_vec_spread(func.ctx.context, 0.),
        "isgainpositive.vec",
    );

    // norm = 1 / (1 + 1 / q * K + K * K)
    let gt_zero_norm = func.ctx.b.build_float_div(
        util::get_vec_spread(func.ctx.context, 1.),
        func.ctx.b.build_float_add(
            func.ctx.b.build_float_add(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx.b.build_float_mul(
                    func.ctx.b.build_float_div(
                        util::get_vec_spread(func.ctx.context, 1.),
                        q_vec,
                        "",
                    ),
                    k_value,
                    "",
                ),
                "",
            ),
            k_squared,
            "",
        ),
        "norm.gtzero",
    );

    // norm = 1 / (1 + V / q * K + K * K)
    let lt_zero_norm = func.ctx.b.build_float_div(
        util::get_vec_spread(func.ctx.context, 1.),
        func.ctx.b.build_float_add(
            func.ctx.b.build_float_add(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx
                    .b
                    .build_float_mul(func.ctx.b.build_float_div(v, q_vec, ""), k_value, ""),
                "",
            ),
            k_squared,
            "",
        ),
        "norm.ltzero",
    );

    // a0 = (1 + V / q * K + K * K) * norm
    let gt_zero_a0 = func.ctx.b.build_float_mul(
        func.ctx.b.build_float_add(
            func.ctx.b.build_float_add(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx
                    .b
                    .build_float_mul(func.ctx.b.build_float_div(v, q_vec, ""), k_value, ""),
                "",
            ),
            k_squared,
            "",
        ),
        gt_zero_norm,
        "a0.gtzero",
    );

    // a0 = (1 + 1 / q * K + K * K) * norm
    let lt_zero_a0 = func.ctx.b.build_float_mul(
        func.ctx.b.build_float_add(
            func.ctx.b.build_float_add(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx.b.build_float_mul(
                    func.ctx.b.build_float_div(
                        util::get_vec_spread(func.ctx.context, 1.),
                        q_vec,
                        "",
                    ),
                    k_value,
                    "",
                ),
                "",
            ),
            k_squared,
            "",
        ),
        lt_zero_norm,
        "a0.ltzero",
    );

    // a1 = 2 * (K * K - 1) * norm
    let gt_zero_a1 = func.ctx.b.build_float_mul(
        func.ctx.b.build_float_mul(
            util::get_vec_spread(func.ctx.context, 2.),
            func.ctx
                .b
                .build_float_sub(k_squared, util::get_vec_spread(func.ctx.context, 1.), ""),
            "",
        ),
        gt_zero_norm,
        "a1.gtzero",
    );

    // a1 = 2 * (K * K - 1) * norm
    let lt_zero_a1 = func.ctx.b.build_float_mul(
        func.ctx.b.build_float_mul(
            util::get_vec_spread(func.ctx.context, 2.),
            func.ctx
                .b
                .build_float_sub(k_squared, util::get_vec_spread(func.ctx.context, 1.), ""),
            "",
        ),
        lt_zero_norm,
        "a1.ltzero",
    );

    // a2 = (1 - V / q * K + K * K) * norm
    let gt_zero_a2 = func.ctx.b.build_float_mul(
        func.ctx.b.build_float_add(
            func.ctx.b.build_float_sub(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx
                    .b
                    .build_float_mul(func.ctx.b.build_float_div(v, q_vec, ""), k_value, ""),
                "",
            ),
            k_squared,
            "",
        ),
        gt_zero_norm,
        "a2.gtzero",
    );

    // a2 = (1 - 1 / q * K + K * K) * norm
    let lt_zero_a2 = func.ctx.b.build_float_mul(
        func.ctx.b.build_float_add(
            func.ctx.b.build_float_sub(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx.b.build_float_mul(
                    func.ctx.b.build_float_div(
                        util::get_vec_spread(func.ctx.context, 1.),
                        q_vec,
                        "",
                    ),
                    k_value,
                    "",
                ),
                "",
            ),
            k_squared,
            "",
        ),
        lt_zero_norm,
        "a2.ltzero",
    );

    // b1 = a1
    let gt_zero_b1 = gt_zero_a1;

    // b1 = a1
    let lt_zero_b1 = lt_zero_a1;

    // b2 = (1 - 1 / q * K + K * K) * norm
    let gt_zero_b2 = func.ctx.b.build_float_mul(
        func.ctx.b.build_float_add(
            func.ctx.b.build_float_sub(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx.b.build_float_mul(
                    func.ctx.b.build_float_div(
                        util::get_vec_spread(func.ctx.context, 1.),
                        q_vec,
                        "",
                    ),
                    k_value,
                    "",
                ),
                "",
            ),
            k_squared,
            "",
        ),
        gt_zero_norm,
        "b2.gtzero",
    );

    // b2 = (1 - V / q * K + K * K) * norm
    let lt_zero_b2 = func.ctx.b.build_float_mul(
        func.ctx.b.build_float_add(
            func.ctx.b.build_float_sub(
                util::get_vec_spread(func.ctx.context, 1.),
                func.ctx
                    .b
                    .build_float_mul(func.ctx.b.build_float_div(v, q_vec, ""), k_value, ""),
                "",
            ),
            k_squared,
            "",
        ),
        lt_zero_norm,
        "b2.ltzero",
    );

    func.ctx.b.build_store(
        &a0_ptr,
        &func
            .ctx
            .b
            .build_select(is_gain_positive_vec, gt_zero_a0, lt_zero_a0, "a0"),
    );
    func.ctx.b.build_store(
        &a1_ptr,
        &func
            .ctx
            .b
            .build_select(is_gain_positive_vec, gt_zero_a1, lt_zero_a1, "a1"),
    );
    func.ctx.b.build_store(
        &a2_ptr,
        &func
            .ctx
            .b
            .build_select(is_gain_positive_vec, gt_zero_a2, lt_zero_a2, "a2"),
    );
    func.ctx.b.build_store(
        &b1_ptr,
        &func
            .ctx
            .b
            .build_select(is_gain_positive_vec, gt_zero_b1, lt_zero_b1, "b1"),
    );
    func.ctx.b.build_store(
        &b2_ptr,
        &func
            .ctx
            .b
            .build_select(is_gain_positive_vec, gt_zero_b2, lt_zero_b2, "b2"),
    );
}
define_biquad_func!(PeakBqFilterFunction: block::Function::PeakBqFilter => true, peak_filter_generate_coefficients);
