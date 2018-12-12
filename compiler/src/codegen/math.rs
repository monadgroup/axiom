use crate::codegen::{build_context_function, globals, util, BuilderContext, TargetProperties};
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicType, VectorType};
use inkwell::values::InstructionOpcode;
use inkwell::values::{FunctionValue, VectorValue};
use inkwell::FloatPredicate;
use std::f64::consts;

// utils
fn get_float_asm_func(module: &Module, asm: &str, constraints: &str) -> FunctionValue {
    let context = module.get_context();
    let fn_type = context.f32_type().fn_type(&[&context.f32_type()], false);
    fn_type.as_asm(asm, constraints, false, false)
}

fn get_i64_spread(context: &Context, val: u64) -> VectorValue {
    VectorType::const_vector(&[
        &context.i64_type().const_int(val, false),
        &context.i64_type().const_int(val, false),
    ])
}

fn get_i32_spread(context: &Context, val: u64) -> VectorValue {
    VectorType::const_vector(&[
        &context.i32_type().const_int(val, false),
        &context.i32_type().const_int(val, false),
        &context.i32_type().const_int(val, false),
        &context.i32_type().const_int(val, false),
    ])
}

pub fn build_math_functions(module: &Module, target: &TargetProperties) {
    build_rand_v2f64(module, target);
    build_sin_v2f64(module, target);
    build_cos_v2f64(module, target);
    build_tan_v2f64(module, target);
    build_floor_v2f64(module, target);
    build_ceil_v2f64(module, target);
    build_round_v2f64(module, target);
    build_mod_v2f64(module, target);
    build_fract_v2f64(module, target);
    build_pow_v2f64(module, target);
    build_exp_v2f64(module, target);
    build_exp2_v2f64(module, target);
    build_exp10_v2f64(module, target);
    build_log_v2f64(module, target);
    build_log2_v2f64(module, target);
    build_log10_v2f64(module, target);
    build_atan2k_v2f64(module, target);
    build_asin_v2f64(module, target);
    build_acos_v2f64(module, target);
    build_atan_v2f64(module, target);
    build_atan2_v2f64(module, target);
    build_sinh_v2f64(module, target);
    build_cosh_v2f64(module, target);
    build_tanh_v2f64(module, target);
    build_hypot_v2f64(module, target);
}

// rand
pub fn rand_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.rand.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (Linkage::PrivateLinkage, v2f64_type.fn_type(&[], false))
    })
}

fn build_rand_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        rand_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let mul_pd_intrinsic =
                util::get_or_create_func(module, "llvm.x86.sse41.pmuldq", true, &|| {
                    (
                        Linkage::ExternalLinkage,
                        ctx.context.i64_type().vec_type(2).fn_type(
                            &[
                                &ctx.context.i32_type().vec_type(4),
                                &ctx.context.i32_type().vec_type(4),
                            ],
                            false,
                        ),
                    )
                });
            let seed_ptr = globals::get_rand_seed(ctx.module).as_pointer_value();

            let current_seed = ctx.b.build_load(&seed_ptr, "seed").into_vector_value();
            let new_seed = ctx
                .b
                .build_call(
                    &mul_pd_intrinsic,
                    &[
                        &ctx.b.build_cast(
                            InstructionOpcode::BitCast,
                            &current_seed,
                            &ctx.context.i32_type().vec_type(4),
                            "",
                        ),
                        &VectorType::const_vector(&[
                            &ctx.context.i32_type().const_int(16007, false),
                            &ctx.context.i32_type().get_undef(),
                            &ctx.context.i32_type().const_int(16007, false),
                            &ctx.context.i32_type().get_undef(),
                        ]),
                    ],
                    "",
                    true,
                )
                .left()
                .unwrap()
                .into_vector_value();
            ctx.b.build_store(&seed_ptr, &new_seed);

            let shuffled = ctx.b.build_signed_int_to_float(
                ctx.b.build_shuffle_vector(
                    &ctx.b
                        .build_cast(
                            InstructionOpcode::BitCast,
                            &new_seed,
                            &ctx.context.i32_type().vec_type(4),
                            "",
                        )
                        .into_vector_value(),
                    &ctx.context.i32_type().vec_type(4).get_undef(),
                    &VectorType::const_vector(&[
                        &ctx.context.i32_type().const_int(2, false),
                        &ctx.context.i32_type().const_int(0, false),
                    ]),
                    "",
                ),
                ctx.context.f64_type().vec_type(2),
                "",
            );
            let normalized = ctx.b.build_float_mul(
                shuffled,
                util::get_vec_spread(ctx.context, 1. / (65536. * 32768.)),
                "",
            );
            ctx.b.build_return(Some(&normalized));
        },
    )
}

// sin + sin_slow
pub fn sin_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.sin.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_sin_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(module, sin_v2f64(module), target, &|ctx: BuilderContext| {
        let mod_intrinsic = mod_v2f64(module);
        let abs_intrinsic = abs_v2f64(module);

        let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

        // keep in the range 0..2pi
        let ranged_x = ctx
            .b
            .build_call(
                &mod_intrinsic,
                &[&x_vec, &util::get_vec_spread(ctx.context, consts::PI * 2.)],
                "x.ranged",
                true,
            )
            .left()
            .unwrap()
            .into_vector_value();

        let wrapped_x = ctx
            .b
            .build_select(
                ctx.b.build_float_compare(
                    FloatPredicate::OGT,
                    ranged_x,
                    util::get_vec_spread(ctx.context, consts::PI),
                    "",
                ),
                ctx.b.build_float_sub(
                    ranged_x,
                    util::get_vec_spread(ctx.context, consts::PI * 2.),
                    "",
                ),
                ranged_x,
                "x.wrapped",
            )
            .into_vector_value();

        let y_val = ctx.b.build_float_add(
            ctx.b.build_float_mul(
                util::get_vec_spread(ctx.context, 1.273_239_544_723_765),
                wrapped_x,
                "",
            ),
            ctx.b.build_float_mul(
                ctx.b.build_float_mul(
                    util::get_vec_spread(ctx.context, -0.405_284_734_566_521_37),
                    wrapped_x,
                    "",
                ),
                ctx.b
                    .build_call(&abs_intrinsic, &[&wrapped_x], "", true)
                    .left()
                    .unwrap()
                    .into_vector_value(),
                "",
            ),
            "y",
        );

        let result = ctx.b.build_float_add(
            ctx.b.build_float_mul(
                util::get_vec_spread(ctx.context, 0.224_999_904_632_568_36),
                ctx.b.build_float_sub(
                    ctx.b.build_float_mul(
                        y_val,
                        ctx.b
                            .build_call(&abs_intrinsic, &[&y_val], "", true)
                            .left()
                            .unwrap()
                            .into_vector_value(),
                        "",
                    ),
                    y_val,
                    "",
                ),
                "",
            ),
            y_val,
            "res",
        );

        ctx.b.build_return(Some(&result));
    })
}

pub fn sin_slow_f64(module: &Module) -> FunctionValue {
    get_float_asm_func(module, "fsin", "={st},0,~{dirflag},~{fpsr},~{flags}")
}

// cos + cos_slow
pub fn cos_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.cos.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_cos_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(module, cos_v2f64(module), target, &|ctx: BuilderContext| {
        let sin_intrinsic = sin_v2f64(module);

        let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

        // cos(x) = sin(x + pi/2)
        let x_offset = ctx.b.build_float_add(
            x_vec,
            util::get_vec_spread(ctx.context, consts::PI / 2.),
            "x.offset",
        );
        let res = ctx
            .b
            .build_call(&sin_intrinsic, &[&x_offset], "", true)
            .left()
            .unwrap()
            .into_vector_value();

        ctx.b.build_return(Some(&res));
    })
}

pub fn cos_slow_f64(module: &Module) -> FunctionValue {
    get_float_asm_func(module, "fcos", "={st},0,~{dirflag},~{fpsr},~{flags}")
}

// tan + tan_slow
pub fn tan_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.tan.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn build_tan_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(module, tan_v2f64(module), target, &|ctx: BuilderContext| {
        let sin_intrinsic = sin_v2f64(module);
        let cos_intrinsic = cos_v2f64(module);

        let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

        // tan(x) = sin(x) / cos(x)
        let sin_x = ctx
            .b
            .build_call(&sin_intrinsic, &[&x_vec], "x.sin", true)
            .left()
            .unwrap()
            .into_vector_value();
        let cos_x = ctx
            .b
            .build_call(&cos_intrinsic, &[&x_vec], "x.cos", true)
            .left()
            .unwrap()
            .into_vector_value();

        let res = ctx.b.build_float_div(sin_x, cos_x, "x.tan");
        ctx.b.build_return(Some(&res));
    })
}

pub fn tan_slow_f64(module: &Module) -> FunctionValue {
    get_float_asm_func(
        module,
        "fptan; fstp $0",
        "={st},0,~{dirflag},~{fpsr},~{flags}",
    )
}

// min
pub fn min_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.x86.sse2.min.pd", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

pub fn min_f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.minnum.f64", true, &|| {
        let f64_type = module.get_context().f64_type();
        (
            Linkage::ExternalLinkage,
            f64_type.fn_type(&[&f64_type, &f64_type], false),
        )
    })
}

// max
pub fn max_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.x86.sse2.max.pd", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

pub fn max_f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.maxnum.f64", true, &|| {
        let f64_type = module.get_context().f64_type();
        (
            Linkage::ExternalLinkage,
            f64_type.fn_type(&[&f64_type, &f64_type], false),
        )
    })
}

// sqrt
pub fn sqrt_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.x86.sse2.sqrt.pd", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

// internal LLVM intrinsic used for rounding
fn sse_round_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.x86.sse41.round.pd", true, &|| {
        let context = module.get_context();
        let v2f64_type = context.f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &context.i32_type()], false),
        )
    })
}

// floor
pub fn floor_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.floor.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_floor_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        floor_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let sse_round_intrinsic = sse_round_v2f64(ctx.module);
            let res = ctx
                .b
                .build_call(
                    &sse_round_intrinsic,
                    &[
                        &ctx.func.get_nth_param(0).unwrap().into_vector_value(),
                        &ctx.context.i32_type().const_int(1, false), // 1 = floor
                    ],
                    "",
                    true,
                )
                .left()
                .unwrap()
                .into_vector_value();
            ctx.b.build_return(Some(&res));
        },
    );
}

// ceil
pub fn ceil_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.ceil.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_ceil_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        ceil_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let sse_round_intrinsic = sse_round_v2f64(ctx.module);
            let res = ctx
                .b
                .build_call(
                    &sse_round_intrinsic,
                    &[
                        &ctx.func.get_nth_param(0).unwrap().into_vector_value(),
                        &ctx.context.i32_type().const_int(2, false), // 2 = ceil
                    ],
                    "",
                    true,
                )
                .left()
                .unwrap()
                .into_vector_value();
            ctx.b.build_return(Some(&res));
        },
    );
}

// round
pub fn round_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.round.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_round_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        round_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let sse_round_intrinsic = sse_round_v2f64(ctx.module);
            let res = ctx
                .b
                .build_call(
                    &sse_round_intrinsic,
                    &[
                        &ctx.func.get_nth_param(0).unwrap().into_vector_value(),
                        &ctx.context.i32_type().const_int(0, false), // 0 = round
                    ],
                    "",
                    true,
                )
                .left()
                .unwrap()
                .into_vector_value();
            ctx.b.build_return(Some(&res));
        },
    );
}

// abs
pub fn abs_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.fabs.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

// copysign
pub fn copysign_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.copysign.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

// mod
// note: this must usage external linkage since blocks can generate code that uses this
// (see gen_math_op.rs)
pub fn mod_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.mod.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

fn build_mod_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(module, mod_v2f64(module), target, &|ctx: BuilderContext| {
        let floor_intrinsic = floor_v2f64(module);

        let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();
        let m_vec = ctx.func.get_nth_param(1).unwrap().into_vector_value();

        let res_vec = ctx.b.build_float_sub(
            x_vec,
            ctx.b.build_float_mul(
                ctx.b
                    .build_call(
                        &floor_intrinsic,
                        &[&ctx.b.build_float_div(x_vec, m_vec, "")],
                        "",
                        true,
                    )
                    .left()
                    .unwrap()
                    .into_vector_value(),
                m_vec,
                "",
            ),
            "",
        );

        ctx.b.build_return(Some(&res_vec));
    });
}

// fract
pub fn fract_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.fract.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_fract_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        fract_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let floor_intrinsic = floor_v2f64(module);

            let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

            let res_vec = ctx.b.build_float_sub(
                x_vec,
                ctx.b
                    .build_call(&floor_intrinsic, &[&x_vec], "", true)
                    .left()
                    .unwrap()
                    .into_vector_value(),
                "",
            );

            ctx.b.build_return(Some(&res_vec));
        },
    )
}

// pow
// note: this must usage external linkage since blocks can generate code that uses this
// (see gen_math_op.rs)
pub fn pow_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.pow.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

fn build_pow_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(module, pow_v2f64(module), target, &|ctx: BuilderContext| {
        let exp2_intrinsic = exp2_v2f64(module);
        let log2_intrinsic = log2_v2f64(module);
        let abs_intrinsic = abs_v2f64(module);
        let mod_intrinsic = mod_v2f64(module);
        let copysign_intrinsic = copysign_v2f64(module);

        let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();
        let y_vec = ctx.func.get_nth_param(1).unwrap().into_vector_value();

        let x_is_zero = ctx.b.build_float_compare(
            FloatPredicate::OEQ,
            x_vec,
            util::get_vec_spread(ctx.context, 0.),
            "iszero",
        );
        let norm_x = ctx
            .b
            .build_select(
                x_is_zero,
                util::get_vec_spread(ctx.context, 1.),
                x_vec,
                "normx",
            )
            .into_vector_value();
        let r = ctx
            .b
            .build_call(
                &exp2_intrinsic,
                &[&ctx.b.build_float_mul(
                    ctx.b
                        .build_call(&log2_intrinsic, &[&norm_x], "", true)
                        .left()
                        .unwrap()
                        .into_vector_value(),
                    y_vec,
                    "",
                )],
                "r",
                true,
            )
            .left()
            .unwrap()
            .into_vector_value();

        // the result is negative if x < 0 and if y is integer and odd
        let abs_y = ctx
            .b
            .build_call(&abs_intrinsic, &[&y_vec], "", true)
            .left()
            .unwrap()
            .into_vector_value();
        let mod_y = ctx
            .b
            .build_call(
                &mod_intrinsic,
                &[&abs_y, &util::get_vec_spread(ctx.context, 2.)],
                "mody",
                true,
            )
            .left()
            .unwrap()
            .into_vector_value();
        let sign = ctx.b.build_float_add(
            ctx.b
                .build_call(&copysign_intrinsic, &[&mod_y, &norm_x], "", true)
                .left()
                .unwrap()
                .into_vector_value(),
            util::get_vec_spread(ctx.context, 0.5),
            "sign",
        );
        let norm_r = ctx
            .b
            .build_call(&copysign_intrinsic, &[&r, &sign], "normr", true)
            .left()
            .unwrap()
            .into_vector_value();

        // handle zero
        let res = ctx
            .b
            .build_select(
                x_is_zero,
                util::get_vec_spread(ctx.context, 0.),
                norm_r,
                "res",
            )
            .into_vector_value();
        ctx.b.build_return(Some(&res));
    })
}

// exp
pub fn exp_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.exp.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_exp_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(module, exp_v2f64(module), target, &|ctx: BuilderContext| {
        let exp2_intrinsic = exp2_v2f64(module);

        let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

        // exp(x) = exp2(x * log_2(e))
        let x_offset = ctx.b.build_float_mul(
            x_vec,
            util::get_vec_spread(ctx.context, consts::LOG2_E),
            "x.offset",
        );
        let res = ctx
            .b
            .build_call(&exp2_intrinsic, &[&x_offset], "", true)
            .left()
            .unwrap()
            .into_vector_value();

        ctx.b.build_return(Some(&res));
    });
}

// exp2
pub fn exp2_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.exp2.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_exp2_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        exp2_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let cvtpd_intrinsic =
                util::get_or_create_func(module, "llvm.x86.sse2.cvtpd2dq", true, &|| {
                    (
                        Linkage::ExternalLinkage,
                        ctx.context
                            .i32_type()
                            .vec_type(4)
                            .fn_type(&[&ctx.context.f64_type().vec_type(2)], false),
                    )
                });

            let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

            let x_int_vec = ctx
                .b
                .build_call(&cvtpd_intrinsic, &[&x_vec], "", true)
                .left()
                .unwrap()
                .into_vector_value();
            let x_int_two_vec = ctx.b.build_shuffle_vector(
                &x_int_vec,
                &ctx.context.i32_type().vec_type(4).get_undef(),
                &VectorType::const_vector(&[
                    &ctx.context.i32_type().const_int(0, false),
                    &ctx.context.i32_type().const_int(1, false),
                ]),
                "",
            );
            let x_frac = ctx.b.build_float_sub(
                x_vec,
                ctx.b.build_signed_int_to_float(
                    x_int_two_vec,
                    ctx.context.f64_type().vec_type(2),
                    "",
                ),
                "x.frac",
            );

            let mad = |r: VectorValue, exp: f64| {
                ctx.b.build_float_add(
                    ctx.b.build_float_mul(r, x_frac, ""),
                    util::get_vec_spread(ctx.context, exp),
                    "",
                )
            };

            let r = util::get_vec_spread(ctx.context, 0.000_154_653_240_841_184_92); // const 0
            let r = mad(r, 0.001_339_529_154_378_738); // const 1
            let r = mad(r, 0.009_618_039_911_742_926); // const 2
            let r = mad(r, 0.055_503_406_540_083_23); // const 3
            let r = mad(r, 0.240_226_511_014_043_35); // const 4
            let r = mad(r, 0.693_147_200_072_417); // const 5
            let r = mad(r, 0.999_999_999_970_896_2); // const 6

            let k = ctx
                .b
                .build_int_add(x_int_vec, get_i32_spread(ctx.context, 1023), "");
            let k = ctx
                .b
                .build_left_shift(k, get_i32_spread(ctx.context, 20), "");
            let k = ctx.b.build_shuffle_vector(
                &k,
                &ctx.context.i32_type().vec_type(4).get_undef(),
                &VectorType::const_vector(&[
                    &ctx.context.i32_type().const_int(2, false),
                    &ctx.context.i32_type().const_int(0, false),
                    &ctx.context.i32_type().const_int(3, false),
                    &ctx.context.i32_type().const_int(1, false),
                ]),
                "",
            );
            let res = ctx.b.build_float_mul(
                r,
                ctx.b
                    .build_cast(
                        InstructionOpcode::BitCast,
                        &k,
                        &ctx.context.f64_type().vec_type(2),
                        "k.float",
                    )
                    .into_vector_value(),
                "",
            );

            ctx.b.build_return(Some(&res));
        },
    )
}

// exp10
pub fn exp10_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.exp10.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_exp10_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        exp10_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let exp2_intrinsic = exp2_v2f64(module);

            let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

            // exp10(x) = exp2(x * log_2(10))
            let x_offset = ctx.b.build_float_mul(
                x_vec,
                util::get_vec_spread(ctx.context, 10_f64.log2()),
                "x.offset",
            );
            let res = ctx
                .b
                .build_call(&exp2_intrinsic, &[&x_offset], "", true)
                .left()
                .unwrap()
                .into_vector_value();

            ctx.b.build_return(Some(&res));
        },
    );
}

// log
pub fn log_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.log.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_log_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(module, log_v2f64(module), target, &|ctx: BuilderContext| {
        let log2_intrinsic = log2_v2f64(module);

        let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

        // log(x) = log2(x) / log2(e)
        let log2_x = ctx
            .b
            .build_call(&log2_intrinsic, &[&x_vec], "", true)
            .left()
            .unwrap()
            .into_vector_value();
        let res = ctx.b.build_float_div(
            log2_x,
            util::get_vec_spread(ctx.context, consts::LOG2_E),
            "",
        );

        ctx.b.build_return(Some(&res));
    })
}

// log2
pub fn log2_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.log2.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_log2_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        log2_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();
            let x_int_vec = ctx
                .b
                .build_cast(
                    InstructionOpcode::BitCast,
                    &x_vec,
                    &ctx.context.i64_type().vec_type(2),
                    "x.int",
                )
                .into_vector_value();

            let min_exp = 1023;
            let exp_o = min_exp << 52;
            let all_bits = u64::max_value();
            let exp_a = all_bits >> 12;

            let min_exp_vec = get_i64_spread(ctx.context, min_exp);

            let ilogb_x = ctx.b.build_shuffle_vector(
                &ctx.b
                    .build_cast(
                        InstructionOpcode::BitCast,
                        &ctx.b.build_int_sub(
                            ctx.b.build_right_shift(
                                x_int_vec,
                                get_i64_spread(ctx.context, 52),
                                false,
                                "",
                            ),
                            min_exp_vec,
                            "",
                        ),
                        &ctx.context.i32_type().vec_type(4),
                        "",
                    )
                    .into_vector_value(),
                &ctx.context.i32_type().vec_type(4).get_undef(),
                &VectorType::const_vector(&[
                    &ctx.context.i32_type().const_int(0, false),
                    &ctx.context.i32_type().const_int(2, false),
                ]),
                "x.ilogb",
            );

            let p_int = ctx.b.build_or(
                ctx.b
                    .build_and(x_int_vec, get_i64_spread(ctx.context, exp_a), ""),
                get_i64_spread(ctx.context, exp_o),
                "p.int",
            );
            let p_float = ctx
                .b
                .build_cast(
                    InstructionOpcode::BitCast,
                    &p_int,
                    &ctx.context.f64_type().vec_type(2),
                    "p.float",
                )
                .into_vector_value();
            let y = ctx.b.build_float_div(
                ctx.b
                    .build_float_sub(p_float, util::get_vec_spread(ctx.context, 1.), ""),
                ctx.b
                    .build_float_add(p_float, util::get_vec_spread(ctx.context, 1.), ""),
                "y",
            );
            let y2 = ctx.b.build_float_mul(y, y, "y2");

            let mad = |r: VectorValue, exp: f64| {
                ctx.b.build_float_add(
                    ctx.b.build_float_mul(r, y2, ""),
                    util::get_vec_spread(ctx.context, exp),
                    "",
                )
            };

            let r = util::get_vec_spread(ctx.context, 0.410_981_538_279_884_26); // const 0
            let r = mad(r, 0.402_155_483_170_645_3); // const 1
            let r = mad(r, 0.577_550_146_270_368_7); // const 2
            let r = mad(r, 0.961_787_806_001_666_5); // const 3
            let r = mad(r, 2.885_390_127_834_398_3); // const 4

            let r = ctx.b.build_float_mul(r, y, "");
            let ilogb_float =
                ctx.b
                    .build_signed_int_to_float(ilogb_x, ctx.context.f64_type().vec_type(2), "");
            let r = ctx.b.build_float_add(r, ilogb_float, "");

            ctx.b.build_return(Some(&r));
        },
    );
}

// log10
pub fn log10_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.log10.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_log10_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        log10_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let log2_intrinsic = log2_v2f64(module);

            let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

            // log10(x) = log2(x) / log2(10)
            let log2_x = ctx
                .b
                .build_call(&log2_intrinsic, &[&x_vec], "", true)
                .left()
                .unwrap()
                .into_vector_value();
            let res =
                ctx.b
                    .build_float_div(log2_x, util::get_vec_spread(ctx.context, 10_f64.log2()), "");

            ctx.b.build_return(Some(&res));
        },
    )
}

// an internal function to support the various a* trig functions
fn atan2k_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.atan2k_v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

fn build_atan2k_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        atan2k_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let abs_intrinsic = abs_v2f64(module);

            let y_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();
            let x_vec = ctx.func.get_nth_param(1).unwrap().into_vector_value();

            let is_x_negative = ctx.b.build_float_compare(
                FloatPredicate::OLT,
                x_vec,
                util::get_vec_spread(ctx.context, 0.),
                "x.negative",
            );
            let q = ctx
                .b
                .build_select(
                    is_x_negative,
                    util::get_vec_spread(ctx.context, -2.),
                    util::get_vec_spread(ctx.context, 0.),
                    "q",
                )
                .into_vector_value();
            let x_vec = ctx
                .b
                .build_call(&abs_intrinsic, &[&x_vec], "x.abs", true)
                .left()
                .unwrap()
                .into_vector_value();
            let cond = ctx
                .b
                .build_float_compare(FloatPredicate::OGT, y_vec, x_vec, "cond");

            let rx = ctx
                .b
                .build_select(cond, y_vec, x_vec, "rx")
                .into_vector_value();
            let ry = ctx
                .b
                .build_select(cond, ctx.b.build_float_neg(&x_vec, ""), y_vec, "ry")
                .into_vector_value();
            let q = ctx.b.build_float_add(
                q,
                ctx.b
                    .build_select(
                        cond,
                        util::get_vec_spread(ctx.context, 1.),
                        util::get_vec_spread(ctx.context, 0.),
                        "",
                    )
                    .into_vector_value(),
                "q",
            );

            let s = ctx.b.build_float_div(ry, rx, "s");
            let t = ctx.b.build_float_mul(s, s, "t");

            let mad = |r: VectorValue, exp: f64| {
                ctx.b.build_float_add(
                    ctx.b.build_float_mul(r, t, ""),
                    util::get_vec_spread(ctx.context, exp),
                    "",
                )
            };

            let r = util::get_vec_spread(ctx.context, 0.002_823_638_962_581_753_7); // const 0
            let r = mad(r, -0.015_956_902_876_496_315); // const 1
            let r = mad(r, 0.042_504_988_610_744_476); // const 2
            let r = mad(r, -0.074_890_092_015_266_42); // const 3
            let r = mad(r, 0.106_347_933_411_598_2); // const 4
            let r = mad(r, -0.142_027_363_181_114_2); // const 5
            let r = mad(r, 0.199_926_957_488_06); // const 6
            let r = mad(r, -0.333_331_018_686_294_56); // const 7

            let t = ctx.b.build_float_add(
                ctx.b
                    .build_float_mul(r, ctx.b.build_float_mul(t, s, ""), ""),
                s,
                "",
            );
            let t = ctx.b.build_float_add(
                ctx.b
                    .build_float_mul(q, util::get_vec_spread(ctx.context, consts::FRAC_PI_2), ""),
                t,
                "",
            );

            ctx.b.build_return(Some(&t));
        },
    )
}

// asin
pub fn asin_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.asin.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_asin_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        asin_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let abs_intrinsic = abs_v2f64(module);
            let sqrt_intrinsic = sqrt_v2f64(module);
            let atan2k_intrinsic = atan2k_v2f64(module);

            let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();
            let x_positive = ctx
                .b
                .build_call(&abs_intrinsic, &[&x_vec], "x.positive", true)
                .left()
                .unwrap()
                .into_vector_value();

            let atan_x = ctx
                .b
                .build_call(
                    &sqrt_intrinsic,
                    &[&ctx.b.build_float_mul(
                        ctx.b
                            .build_float_add(util::get_vec_spread(ctx.context, 1.), x_vec, ""),
                        ctx.b
                            .build_float_sub(util::get_vec_spread(ctx.context, 1.), x_vec, ""),
                        "",
                    )],
                    "x.atan",
                    true,
                )
                .left()
                .unwrap()
                .into_vector_value();
            let atan_val = ctx
                .b
                .build_call(&atan2k_intrinsic, &[&x_positive, &atan_x], "atan", true)
                .left()
                .unwrap()
                .into_vector_value();

            // if x < 0, multiply atan by -1
            let x_is_negative = ctx.b.build_float_compare(
                FloatPredicate::OLT,
                x_vec,
                util::get_vec_spread(ctx.context, 0.),
                "x.negative",
            );
            let atan_mul = ctx
                .b
                .build_select(
                    x_is_negative,
                    util::get_vec_spread(ctx.context, -1.),
                    util::get_vec_spread(ctx.context, 1.),
                    "atan.mul",
                )
                .into_vector_value();
            let res = ctx.b.build_float_mul(atan_val, atan_mul, "");
            ctx.b.build_return(Some(&res));
        },
    )
}

// acos
pub fn acos_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.acos.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_acos_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        acos_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let abs_intrinsic = abs_v2f64(module);
            let sqrt_intrinsic = sqrt_v2f64(module);
            let atan2k_intrinsic = atan2k_v2f64(module);

            let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();
            let x_positive = ctx
                .b
                .build_call(&abs_intrinsic, &[&x_vec], "x.positive", true)
                .left()
                .unwrap()
                .into_vector_value();

            let atan_x = ctx
                .b
                .build_call(
                    &sqrt_intrinsic,
                    &[&ctx.b.build_float_mul(
                        ctx.b
                            .build_float_add(util::get_vec_spread(ctx.context, 1.), x_vec, ""),
                        ctx.b
                            .build_float_sub(util::get_vec_spread(ctx.context, 1.), x_vec, ""),
                        "",
                    )],
                    "x.atan",
                    true,
                )
                .left()
                .unwrap()
                .into_vector_value();
            let atan_val = ctx
                .b
                .build_call(&atan2k_intrinsic, &[&atan_x, &x_positive], "atan", true)
                .left()
                .unwrap()
                .into_vector_value();

            // if x < 0, multiply atan by -1
            let x_is_negative = ctx.b.build_float_compare(
                FloatPredicate::OLT,
                x_vec,
                util::get_vec_spread(ctx.context, 0.),
                "x.negative",
            );
            let atan_mul = ctx
                .b
                .build_select(
                    x_is_negative,
                    util::get_vec_spread(ctx.context, -1.),
                    util::get_vec_spread(ctx.context, 1.),
                    "atan.mul",
                )
                .into_vector_value();
            let res = ctx.b.build_float_mul(atan_val, atan_mul, "");
            let res = ctx.b.build_float_add(
                res,
                ctx.b
                    .build_select(
                        x_is_negative,
                        util::get_vec_spread(ctx.context, consts::PI),
                        util::get_vec_spread(ctx.context, 0.),
                        "",
                    )
                    .into_vector_value(),
                "",
            );
            ctx.b.build_return(Some(&res));
        },
    )
}

// atan
pub fn atan_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.atan.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_atan_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        atan_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let abs_intrinsic = abs_v2f64(module);
            let copysign_intrinsic = copysign_v2f64(module);

            let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

            let s = ctx
                .b
                .build_call(&abs_intrinsic, &[&x_vec], "x.abs", true)
                .left()
                .unwrap()
                .into_vector_value();
            let q0 = ctx.b.build_float_compare(
                FloatPredicate::OGT,
                s,
                util::get_vec_spread(ctx.context, 1.),
                "q0",
            );
            let s = ctx
                .b
                .build_select(
                    q0,
                    ctx.b
                        .build_float_div(util::get_vec_spread(ctx.context, 1.), s, ""),
                    s,
                    "",
                )
                .into_vector_value();
            let t = ctx.b.build_float_mul(s, s, "");

            let mad = |r: VectorValue, exp: f64| {
                ctx.b.build_float_add(
                    ctx.b.build_float_mul(r, t, ""),
                    util::get_vec_spread(ctx.context, exp),
                    "",
                )
            };

            let r = util::get_vec_spread(ctx.context, 0.002_823_638_962_581_753_7); // const 0
            let r = mad(r, -0.015_956_902_876_496_315); // const 1
            let r = mad(r, 0.042_504_988_610_744_476); // const 2
            let r = mad(r, -0.074_890_092_015_266_42); // const 3
            let r = mad(r, 0.106_347_933_411_598_2); // const 4
            let r = mad(r, -0.142_027_363_181_114_2); // const 5
            let r = mad(r, 0.199_926_957_488_06); // const 6
            let r = mad(r, -0.333_331_018_686_294_56); // const 7

            let t = ctx.b.build_float_add(
                s,
                ctx.b
                    .build_float_mul(s, ctx.b.build_float_mul(t, r, ""), ""),
                "",
            );
            let t = ctx
                .b
                .build_select(
                    q0,
                    ctx.b.build_float_sub(
                        util::get_vec_spread(ctx.context, consts::FRAC_PI_2),
                        t,
                        "",
                    ),
                    t,
                    "",
                )
                .into_vector_value();
            let res = ctx
                .b
                .build_call(&copysign_intrinsic, &[&t, &x_vec], "", true)
                .left()
                .unwrap()
                .into_vector_value();

            ctx.b.build_return(Some(&res));
        },
    )
}

// atan2
pub fn atan2_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.atan2.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

fn build_atan2_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        atan2_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let abs_intrinsic = abs_v2f64(module);
            let atan2k_intrinsic = atan2k_v2f64(module);

            let y_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();
            let x_vec = ctx.func.get_nth_param(1).unwrap().into_vector_value();

            let abs_y = ctx
                .b
                .build_call(&abs_intrinsic, &[&y_vec], "y.abs", true)
                .left()
                .unwrap()
                .into_vector_value();

            let r = ctx
                .b
                .build_call(&atan2k_intrinsic, &[&abs_y, &x_vec], "r", true)
                .left()
                .unwrap()
                .into_vector_value();

            // multiply r by -1 if x < 0
            let x_is_negative = ctx.b.build_float_compare(
                FloatPredicate::OLT,
                x_vec,
                util::get_vec_spread(ctx.context, 0.),
                "x.negative",
            );
            let r_mult = ctx
                .b
                .build_select(
                    x_is_negative,
                    util::get_vec_spread(ctx.context, -1.),
                    util::get_vec_spread(ctx.context, 1.),
                    "r.mult",
                )
                .into_vector_value();
            let r = ctx.b.build_float_mul(r, r_mult, "r");

            // note: we ignore the case where X or Y are infinite
            let x_is_zero = ctx.b.build_float_compare(
                FloatPredicate::OEQ,
                x_vec,
                util::get_vec_spread(ctx.context, 0.),
                "x.zero",
            );
            let y_is_zero = ctx.b.build_float_compare(
                FloatPredicate::OEQ,
                y_vec,
                util::get_vec_spread(ctx.context, 0.),
                "y.zero",
            );

            let r = ctx
                .b
                .build_select(
                    x_is_zero,
                    util::get_vec_spread(ctx.context, consts::FRAC_PI_2),
                    r,
                    "r",
                )
                .into_vector_value();
            let r = ctx
                .b
                .build_select(
                    y_is_zero,
                    ctx.b
                        .build_select(
                            x_is_negative,
                            util::get_vec_spread(ctx.context, consts::PI),
                            util::get_vec_spread(ctx.context, 0.),
                            "",
                        )
                        .into_vector_value(),
                    r,
                    "",
                )
                .into_vector_value();

            // multiply r by -1 if y < 0
            let y_is_negative = ctx.b.build_float_compare(
                FloatPredicate::OLT,
                y_vec,
                util::get_vec_spread(ctx.context, 0.),
                "y.negative",
            );
            let r_mult = ctx
                .b
                .build_select(
                    y_is_negative,
                    util::get_vec_spread(ctx.context, -1.),
                    util::get_vec_spread(ctx.context, 1.),
                    "r.mult",
                )
                .into_vector_value();
            let r = ctx.b.build_float_mul(r, r_mult, "");

            ctx.b.build_return(Some(&r));
        },
    )
}

// sinh
pub fn sinh_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.sinh.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_sinh_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        sinh_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let exp_intrinsic = exp_v2f64(module);

            let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

            let res = ctx.b.build_float_mul(
                ctx.b.build_float_sub(
                    ctx.b
                        .build_call(&exp_intrinsic, &[&x_vec], "", true)
                        .left()
                        .unwrap()
                        .into_vector_value(),
                    ctx.b
                        .build_call(
                            &exp_intrinsic,
                            &[&ctx.b.build_float_neg(&x_vec, "")],
                            "",
                            true,
                        )
                        .left()
                        .unwrap()
                        .into_vector_value(),
                    "",
                ),
                util::get_vec_spread(ctx.context, 0.5),
                "",
            );
            ctx.b.build_return(Some(&res));
        },
    )
}

// cosh
pub fn cosh_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.cosh.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_cosh_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        cosh_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let exp_intrinsic = exp_v2f64(module);

            let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

            let res = ctx.b.build_float_mul(
                ctx.b.build_float_add(
                    ctx.b
                        .build_call(&exp_intrinsic, &[&x_vec], "", true)
                        .left()
                        .unwrap()
                        .into_vector_value(),
                    ctx.b
                        .build_call(
                            &exp_intrinsic,
                            &[&ctx.b.build_float_neg(&x_vec, "")],
                            "",
                            true,
                        )
                        .left()
                        .unwrap()
                        .into_vector_value(),
                    "",
                ),
                util::get_vec_spread(ctx.context, 0.5),
                "",
            );
            ctx.b.build_return(Some(&res));
        },
    )
}

// tanh
pub fn tanh_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.tanh.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

fn build_tanh_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        tanh_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let sinh_intrinsic = sinh_v2f64(module);
            let cosh_intrinsic = cosh_v2f64(module);

            let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

            let res = ctx.b.build_float_div(
                ctx.b
                    .build_call(&sinh_intrinsic, &[&x_vec], "x.sinh", true)
                    .left()
                    .unwrap()
                    .into_vector_value(),
                ctx.b
                    .build_call(&cosh_intrinsic, &[&x_vec], "x.cosh", true)
                    .left()
                    .unwrap()
                    .into_vector_value(),
                "",
            );
            ctx.b.build_return(Some(&res));
        },
    )
}

// hypot
pub fn hypot_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.hypot.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

fn build_hypot_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        hypot_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let sqrt_intrinsic = sqrt_v2f64(module);

            let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();
            let y_vec = ctx.func.get_nth_param(1).unwrap().into_vector_value();

            let x2_vec = ctx.b.build_float_mul(x_vec, x_vec, "x.2");
            let y2_vec = ctx.b.build_float_mul(y_vec, y_vec, "y.2");
            let sum_vec = ctx.b.build_float_add(x2_vec, y2_vec, "xy");

            let sqrt_vec = ctx
                .b
                .build_call(&sqrt_intrinsic, &[&sum_vec], "", true)
                .left()
                .unwrap()
                .into_vector_value();
            ctx.b.build_return(Some(&sqrt_vec));
        },
    )
}
