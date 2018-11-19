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
    ])
}

pub fn build_math_functions(module: &Module, target: &TargetProperties) {
    build_rand_v2f64(module, target);
    build_sin_v2f64(module, target);
    build_cos_v2f64(module, target);
    build_mod_v2f64(module, target);
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
            Linkage::ExternalLinkage,
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
    // todo
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
    // todo
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
    // todo
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
    // todo
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
