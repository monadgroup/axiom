use codegen::{build_context_function, util, BuilderContext, TargetProperties};
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::targets::TargetData;
use inkwell::types::{BasicType, VectorType};
use inkwell::values::InstructionOpcode;
use inkwell::values::{FunctionValue, VectorValue};
use inkwell::FloatPredicate;
use std::f64::consts;

// utils
fn get_float_asm_func(module: &Module, asm: &str, constraints: &str) -> FunctionValue {
    let context = module.get_context();
    let fn_type = context.f32_type().fn_type(&[&context.f32_type()], false);
    let asm_val = fn_type.as_asm(asm, constraints, false, false);
    asm_val
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
    build_mod_v2f64(module, target);
    build_frac_v2f64(module, target);
    build_pow_v2f64(module, target);
    build_exp_v2f64(module, target);
    build_exp2_v2f64(module, target);
    build_exp10_v2f64(module, target);
    build_log_v2f64(module, target);
    build_log2_v2f64(module, target);
    build_log10_v2f64(module, target);
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
    // todo
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
            ).left()
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
            ).into_vector_value();

        let y_val = ctx.b.build_float_add(
            ctx.b.build_float_mul(
                util::get_vec_spread(ctx.context, 1.2732395447237650),
                wrapped_x,
                "",
            ),
            ctx.b.build_float_mul(
                ctx.b.build_float_mul(
                    util::get_vec_spread(ctx.context, -0.40528473456652137),
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
                util::get_vec_spread(ctx.context, 0.22499990463256836),
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

// tan
pub fn tan_f64(module: &Module) -> FunctionValue {
    get_float_asm_func(
        module,
        "fptan; fstp $0",
        "={st},0,~{dirflag},~{fpsr},~{flags}",
    )
}

// min
pub fn min_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.minnum.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

// max
pub fn max_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.maxnum.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

// sqrt
pub fn sqrt_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.sqrt.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

// floor
pub fn floor_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.floor.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

// ceil
pub fn ceil_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.ceil.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

// round
pub fn round_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.round.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
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
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

// mod
pub fn mod_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.mod.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
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
                    ).left()
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

// frac
pub fn frac_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.frac.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

fn build_frac_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        frac_v2f64(module),
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
pub fn pow_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.pow.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

fn build_pow_v2f64(module: &Module, target: &TargetProperties) {
    // todo
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

            let r = util::get_vec_spread(ctx.context, 0.00015465324084118492); // const 0
            let r = mad(r, 0.0013395291543787380); // const 1
            let r = mad(r, 0.0096180399117429261); // const 2
            let r = mad(r, 0.055503406540083233); // const 3
            let r = mad(r, 0.24022651101404335); // const 4
            let r = mad(r, 0.69314720007241704); // const 5
            let r = mad(r, 0.99999999997089617); // const 6

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
                    ).into_vector_value(),
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
    // todo
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
    // todo
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
    // todo
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
    // todo
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
    // todo
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
    // todo
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
    // todo
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
    // todo
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
    // todo
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
    // todo
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
    // todo
}
