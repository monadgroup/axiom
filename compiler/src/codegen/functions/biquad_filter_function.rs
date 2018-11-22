use super::{Function, FunctionContext, VarArgs};
use codegen::values::NumValue;
use codegen::{build_context_function, globals, math, util, BuilderContext, TargetProperties};
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicType, StructType};
use inkwell::values::{FunctionValue, PointerValue, VectorValue};
use inkwell::AddressSpace;
use inkwell::FloatPredicate;
use mir::block;
use std::f64::consts;

fn get_internal_biquad_func(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.util.biquad.biquadFilter", true, &|| {
        let context = &module.get_context();
        let float_vec = context.f64_type().vec_type(2);
        let func = float_vec.fn_type(
            &[
                &float_vec,                                 // input
                &float_vec,                                 // a1
                &float_vec,                                 // a2
                &float_vec,                                 // b0
                &float_vec,                                 // b1
                &float_vec,                                 // b2
                &float_vec.ptr_type(AddressSpace::Generic), // y1
                &float_vec.ptr_type(AddressSpace::Generic), // y2
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
        let a1_vec = ctx.func.get_nth_param(1).unwrap().into_vector_value();
        let a2_vec = ctx.func.get_nth_param(2).unwrap().into_vector_value();
        let b0_vec = ctx.func.get_nth_param(3).unwrap().into_vector_value();
        let b1_vec = ctx.func.get_nth_param(4).unwrap().into_vector_value();
        let b2_vec = ctx.func.get_nth_param(5).unwrap().into_vector_value();
        let y1_vec_ptr = ctx.func.get_nth_param(6).unwrap().into_pointer_value();
        let y2_vec_ptr = ctx.func.get_nth_param(7).unwrap().into_pointer_value();
        let z1_vec_ptr = ctx.func.get_nth_param(8).unwrap().into_pointer_value();
        let z2_vec_ptr = ctx.func.get_nth_param(9).unwrap().into_pointer_value();

        // output = (b0 * in) + (b1 * y1) + (b2 * y2) - (a1 * z1) - (a2 * z2)
        let out_1 = ctx.b.build_float_mul(b0_vec, input_vec, "");
        let out_2 = ctx.b.build_float_mul(
            b1_vec,
            ctx.b.build_load(&y1_vec_ptr, "").into_vector_value(),
            "",
        );
        let out_3 = ctx.b.build_float_mul(
            b2_vec,
            ctx.b.build_load(&y2_vec_ptr, "").into_vector_value(),
            "",
        );
        let out_4 = ctx.b.build_float_mul(
            a1_vec,
            ctx.b.build_load(&z1_vec_ptr, "").into_vector_value(),
            "",
        );
        let out_5 = ctx.b.build_float_mul(
            a2_vec,
            ctx.b.build_load(&z2_vec_ptr, "").into_vector_value(),
            "",
        );
        let output_vec = ctx.b.build_float_sub(
            ctx.b.build_float_sub(
                ctx.b
                    .build_float_add(ctx.b.build_float_add(out_1, out_2, ""), out_3, ""),
                out_4,
                "",
            ),
            out_5,
            "",
        );

        // y2 = y1
        ctx.b.build_store(
            &y2_vec_ptr,
            &ctx.b.build_load(&y1_vec_ptr, "").into_vector_value(),
        );

        // y1 = in
        ctx.b.build_store(&y1_vec_ptr, &input_vec);

        // z2 = z1
        ctx.b.build_store(
            &z2_vec_ptr,
            &ctx.b.build_load(&z1_vec_ptr, "").into_vector_value(),
        );

        // z1 = out
        ctx.b.build_store(&z1_vec_ptr, &output_vec);

        ctx.b.build_return(Some(&output_vec));
    });
}

fn biquad_data_type(context: &Context, has_gain: bool) -> StructType {
    let vec_type = context.f64_type().vec_type(2);
    let mut field_types: Vec<&BasicType> = vec![
        &vec_type, // a1
        &vec_type, // a2
        &vec_type, // b0
        &vec_type, // b1
        &vec_type, // b2
        &vec_type, // y1 (previous input 1)
        &vec_type, // y2 (previous input 2)
        &vec_type, // z1 (previous output 1)
        &vec_type, // z2 (previous output 2)
        &vec_type, // cached frequency
        &vec_type, // cached Q
    ];
    if has_gain {
        field_types.push(&vec_type);
    }

    context.struct_type(&field_types, false)
}

struct Coefficients {
    a0: VectorValue,
    a1: VectorValue,
    a2: VectorValue,
    b0: VectorValue,
    b1: VectorValue,
    b2: VectorValue,
}

type GenerateCoefficientsFn =
    Fn(&mut FunctionContext, VectorValue, VectorValue, Option<VectorValue>) -> Coefficients;

fn gen_biquad_call(
    func: &mut FunctionContext,
    args: &[PointerValue],
    result: PointerValue,
    has_gain: bool,
    generate_coefficients: &GenerateCoefficientsFn,
) {
    let max_intrinsic = math::max_v2f64(func.ctx.module);
    let sin_intrinsic = math::sin_v2f64(func.ctx.module);
    let cos_intrinsic = math::cos_v2f64(func.ctx.module);
    let internal_biquad_func = get_internal_biquad_func(func.ctx.module);

    let a1_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 0, "a1.ptr") };
    let a2_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 1, "a2.ptr") };
    let b0_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 2, "b0.ptr") };
    let b1_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 3, "b1.ptr") };
    let b2_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 4, "b2.ptr") };
    let y1_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 5, "y1.ptr") };
    let y2_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 6, "y2.ptr") };
    let z1_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 7, "z1.ptr") };
    let z2_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 8, "z2.ptr") };
    let cached_freq_ptr = unsafe {
        func.ctx
            .b
            .build_struct_gep(&func.data_ptr, 9, "cachedfreq.ptr")
    };
    let cached_q_ptr = unsafe {
        func.ctx
            .b
            .build_struct_gep(&func.data_ptr, 10, "cachedq.ptr")
    };
    let cached_gain_ptr = if has_gain {
        Some(unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 11, "cachedgain.ptr")
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
            true,
        ).left()
        .unwrap()
        .into_vector_value();

    // ensure the frequency is 0.01 or above to avoid filter problems around very low frequencies
    let f0 = func
        .ctx
        .b
        .build_call(
            &max_intrinsic,
            &[&freq_vec, &util::get_vec_spread(func.ctx.context, 0.01)],
            "",
            true,
        ).left()
        .unwrap()
        .into_vector_value();

    // w0 = 2 * PI * f0 / fs
    let fs = func
        .ctx
        .b
        .build_load(
            &globals::get_sample_rate(func.ctx.module).as_pointer_value(),
            "samplerate",
        ).into_vector_value();
    let w0 = func.ctx.b.build_float_mul(
        util::get_vec_spread(func.ctx.context, 2. * consts::PI),
        func.ctx.b.build_float_div(f0, fs, ""),
        "w0",
    );

    // alpha = sin(w0) / (2q)
    let alpha = func.ctx.b.build_float_div(
        func.ctx
            .b
            .build_call(&sin_intrinsic, &[&w0], "", true)
            .left()
            .unwrap()
            .into_vector_value(),
        func.ctx
            .b
            .build_float_mul(util::get_vec_spread(func.ctx.context, 2.), q_vec, ""),
        "alpha",
    );

    // cos_w0 = cos(w0)
    let cos_w0 = func
        .ctx
        .b
        .build_call(&cos_intrinsic, &[&w0], "", true)
        .left()
        .unwrap()
        .into_vector_value();

    let coefficients = generate_coefficients(func, cos_w0, alpha, gain_vec);

    // divide each value by a0 and store
    func.ctx.b.build_store(
        &a1_ptr,
        &func
            .ctx
            .b
            .build_float_div(coefficients.a1, coefficients.a0, ""),
    );
    func.ctx.b.build_store(
        &a2_ptr,
        &func
            .ctx
            .b
            .build_float_div(coefficients.a2, coefficients.a0, ""),
    );
    func.ctx.b.build_store(
        &b0_ptr,
        &func
            .ctx
            .b
            .build_float_div(coefficients.b0, coefficients.a0, ""),
    );
    func.ctx.b.build_store(
        &b1_ptr,
        &func
            .ctx
            .b
            .build_float_div(coefficients.b1, coefficients.a0, ""),
    );
    func.ctx.b.build_store(
        &b2_ptr,
        &func
            .ctx
            .b
            .build_float_div(coefficients.b2, coefficients.a0, ""),
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
                &func.ctx.b.build_load(&a1_ptr, "a1").into_vector_value(),
                &func.ctx.b.build_load(&a2_ptr, "a2").into_vector_value(),
                &func.ctx.b.build_load(&b0_ptr, "b0").into_vector_value(),
                &func.ctx.b.build_load(&b1_ptr, "b1").into_vector_value(),
                &func.ctx.b.build_load(&b2_ptr, "b2").into_vector_value(),
                &y1_ptr,
                &y2_ptr,
                &z1_ptr,
                &z2_ptr,
            ],
            "resultvec",
            true,
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
    cos_w0: VectorValue,
    alpha: VectorValue,
    _gain_vec: Option<VectorValue>,
) -> Coefficients {
    let one_minus_cos_w0 =
        func.ctx
            .b
            .build_float_sub(util::get_vec_spread(func.ctx.context, 1.), cos_w0, "");

    // b0 = (1 - cos_w0) / 2
    let b0 = func.ctx.b.build_float_div(
        one_minus_cos_w0,
        util::get_vec_spread(func.ctx.context, 2.),
        "b0",
    );

    // b1 = 1 - cos_w0
    let b1 = one_minus_cos_w0;

    // b2 = (1 - cos_w0) / 2
    let b2 = b0;

    // a0 = 1 + alpha
    let a0 = func
        .ctx
        .b
        .build_float_add(util::get_vec_spread(func.ctx.context, 1.), alpha, "a0");

    // a1 = -2 * cos_w0
    let a1 = func
        .ctx
        .b
        .build_float_mul(util::get_vec_spread(func.ctx.context, -2.), cos_w0, "a1");

    // a2 = 1 - alpha
    let a2 = func
        .ctx
        .b
        .build_float_sub(util::get_vec_spread(func.ctx.context, 1.), alpha, "a2");

    Coefficients {
        b0,
        b1,
        b2,
        a0,
        a1,
        a2,
    }
}
define_biquad_func!(LowBqFilterFunction: block::Function::LowBqFilter => false, low_filter_generate_coefficients);

fn high_filter_generate_coefficients(
    func: &mut FunctionContext,
    cos_w0: VectorValue,
    alpha: VectorValue,
    _gain_vec: Option<VectorValue>,
) -> Coefficients {
    let one_plus_cos_w0 =
        func.ctx
            .b
            .build_float_add(util::get_vec_spread(func.ctx.context, 1.), cos_w0, "");

    // b0 = (1 + cos_w0) / 2
    let b0 = func.ctx.b.build_float_div(
        one_plus_cos_w0,
        util::get_vec_spread(func.ctx.context, 2.),
        "b0",
    );

    // b1 = -(1 + cos_w0)
    let b1 = func.ctx.b.build_float_neg(&one_plus_cos_w0, "b1");

    // b2 = (1 + cos_w0) / 2
    let b2 = b0;

    // a0 = 1 + alpha
    let a0 = func
        .ctx
        .b
        .build_float_add(util::get_vec_spread(func.ctx.context, 1.), alpha, "a0");

    // a1 = -2 * cos_w0
    let a1 = func
        .ctx
        .b
        .build_float_mul(util::get_vec_spread(func.ctx.context, -2.), cos_w0, "a1");

    // a2 = 1 - alpha
    let a2 = func
        .ctx
        .b
        .build_float_sub(util::get_vec_spread(func.ctx.context, 1.), alpha, "a2");

    Coefficients {
        b0,
        b1,
        b2,
        a0,
        a1,
        a2,
    }
}
define_biquad_func!(HighBqFilterFunction: block::Function::HighBqFilter => false, high_filter_generate_coefficients);

fn band_filter_generate_coefficients(
    func: &mut FunctionContext,
    cos_w0: VectorValue,
    alpha: VectorValue,
    _gain_vec: Option<VectorValue>,
) -> Coefficients {
    // b0 = alpha
    let b0 = alpha;

    // b1 = 0
    let b1 = util::get_vec_spread(func.ctx.context, 0.);

    // b2 = -alpha
    let b2 = func.ctx.b.build_float_neg(&alpha, "b2");

    // a0 = 1 + alpha
    let a0 = func
        .ctx
        .b
        .build_float_add(util::get_vec_spread(func.ctx.context, 1.), alpha, "a0");

    // a1 = -2 * cos_w0
    let a1 = func
        .ctx
        .b
        .build_float_mul(util::get_vec_spread(func.ctx.context, -2.), cos_w0, "a1");

    // a2 = 1 - alpha
    let a2 = func
        .ctx
        .b
        .build_float_sub(util::get_vec_spread(func.ctx.context, 1.), alpha, "a2");

    Coefficients {
        b0,
        b1,
        b2,
        a0,
        a1,
        a2,
    }
}
define_biquad_func!(BandBqFilterFunction: block::Function::BandBqFilter => false, band_filter_generate_coefficients);

fn notch_filter_generate_coefficients(
    func: &mut FunctionContext,
    cos_w0: VectorValue,
    alpha: VectorValue,
    _gain_vec: Option<VectorValue>,
) -> Coefficients {
    // b0 = 1
    let b0 = util::get_vec_spread(func.ctx.context, 1.);

    // b1 = -2 * cos_w0
    let b1 = func
        .ctx
        .b
        .build_float_mul(util::get_vec_spread(func.ctx.context, -2.), cos_w0, "b1");

    // b2 = 1
    let b2 = util::get_vec_spread(func.ctx.context, 1.);

    // a0 = 1 + alpha
    let a0 = func
        .ctx
        .b
        .build_float_add(util::get_vec_spread(func.ctx.context, 1.), alpha, "a0");

    // a1 = -2 * cos_w0
    let a1 = func
        .ctx
        .b
        .build_float_mul(util::get_vec_spread(func.ctx.context, -2.), cos_w0, "a1");

    // a2 = 1 - alpha
    let a2 = func
        .ctx
        .b
        .build_float_sub(util::get_vec_spread(func.ctx.context, 1.), alpha, "a2");

    Coefficients {
        b0,
        b1,
        b2,
        a0,
        a1,
        a2,
    }
}
define_biquad_func!(NotchBqFilterFunction: block::Function::NotchBqFilter => false, notch_filter_generate_coefficients);

fn all_filter_generate_coefficients(
    func: &mut FunctionContext,
    cos_w0: VectorValue,
    alpha: VectorValue,
    _gain_vec: Option<VectorValue>,
) -> Coefficients {
    // b0 = 1 - alpha
    let b0 = func
        .ctx
        .b
        .build_float_sub(util::get_vec_spread(func.ctx.context, 1.), alpha, "b0");

    // b1 = -2 * cos_w0
    let b1 = func
        .ctx
        .b
        .build_float_mul(util::get_vec_spread(func.ctx.context, -2.), cos_w0, "b1");

    // b2 = 1 + alpha
    let b2 = func
        .ctx
        .b
        .build_float_add(util::get_vec_spread(func.ctx.context, 1.), alpha, "b2");

    // a0 = 1 + alpha
    let a0 = b2;

    // a1 = -2 * cos_w0
    let a1 = b1;

    // a2 = 1 - alpha
    let a2 = b0;

    Coefficients {
        b0,
        b1,
        b2,
        a0,
        a1,
        a2,
    }
}
define_biquad_func!(AllBqFilterFunction: block::Function::AllBqFilter => false, all_filter_generate_coefficients);

fn peak_filter_generate_coefficients(
    func: &mut FunctionContext,
    cos_w0: VectorValue,
    alpha: VectorValue,
    gain_vec: Option<VectorValue>,
) -> Coefficients {
    let max_intrinsic = math::max_v2f64(func.ctx.module);

    let gain_vec = gain_vec.unwrap();

    // clamp gain a bit so we don't get a 0 value
    let clamped_gain = func
        .ctx
        .b
        .build_call(
            &max_intrinsic,
            &[&gain_vec, &util::get_vec_spread(func.ctx.context, 0.001)],
            "",
            true,
        ).left()
        .unwrap()
        .into_vector_value();

    let alpha_mul_gain = func.ctx.b.build_float_mul(alpha, clamped_gain, "");
    let alpha_div_gain = func.ctx.b.build_float_div(alpha, clamped_gain, "");

    // b0 = 1 + alpha * gain
    let b0 = func.ctx.b.build_float_add(
        util::get_vec_spread(func.ctx.context, 1.),
        alpha_mul_gain,
        "b0",
    );

    // b1 = -2 * cos_w0
    let b1 = func
        .ctx
        .b
        .build_float_mul(util::get_vec_spread(func.ctx.context, -2.), cos_w0, "b1");

    // b2 = 1 - alpha * gain
    let b2 = func.ctx.b.build_float_sub(
        util::get_vec_spread(func.ctx.context, 1.),
        alpha_mul_gain,
        "b2",
    );

    // a0 = 1 + alpha / gain
    let a0 = func.ctx.b.build_float_add(
        util::get_vec_spread(func.ctx.context, 1.),
        alpha_div_gain,
        "a0",
    );

    // a1 = -2 * cos_w0
    let a1 = func
        .ctx
        .b
        .build_float_mul(util::get_vec_spread(func.ctx.context, -2.), cos_w0, "a1");

    // a2 = 1 - alpha / gain
    let a2 = func.ctx.b.build_float_sub(
        util::get_vec_spread(func.ctx.context, 1.),
        alpha_div_gain,
        "a2",
    );

    Coefficients {
        b0,
        b1,
        b2,
        a0,
        a1,
        a2,
    }
}
define_biquad_func!(PeakBqFilterFunction: block::Function::PeakBqFilter => true, peak_filter_generate_coefficients);
