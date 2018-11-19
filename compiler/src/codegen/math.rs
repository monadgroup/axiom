use codegen::{build_context_function, globals, util, BuilderContext, TargetProperties};
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::targets::TargetData;
use inkwell::types::{BasicType, VectorType};
use inkwell::values::InstructionOpcode;
use inkwell::values::{FunctionValue, VectorValue};
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
    build_sin_v2f64(module, target);
}

// rand
pub fn rand_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.rand.v2f64", true, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (Linkage::PrivateLinkage, v2f64_type.fn_type(&[], false))
    })
}

fn build_rand_v2f64(module: &Module) {
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
        let abs_intrinsic = abs_v2f64(module);

        let sine_table_ptr = globals::get_sine_table(ctx.module).as_pointer_value();

        let x_vec = ctx.func.get_nth_param(0).unwrap().into_vector_value();

        // sine is symmetrical around zero, so we can get rid of negatives
        let positive_x_vec = ctx
            .b
            .build_call(&abs_intrinsic, &[&x_vec], "x.positive", true)
            .left()
            .unwrap()
            .into_vector_value();

        // normalize range from 0 -> 2PI to 1 -> 2
        let normalized_x = ctx.b.build_float_add(
            ctx.b.build_float_mul(
                positive_x_vec,
                util::get_vec_spread(ctx.context, 1. / (2. * consts::PI)),
                "",
            ),
            util::get_vec_spread(ctx.context, 1.),
            "x.normalized",
        );

        // read the values as integers to calculate approximate indices
        let x_int = ctx
            .b
            .build_cast(
                InstructionOpcode::BitCast,
                &normalized_x,
                &ctx.context.i64_type().vec_type(2),
                "x.int",
            ).into_vector_value();
        let exponent = ctx.b.build_int_sub(
            ctx.b
                .build_right_shift(x_int, get_i64_spread(ctx.context, 52), false, ""),
            get_i64_spread(ctx.context, 1023),
            "exponent",
        );

        let fract_bits = 32 - globals::SINE_TABLE_LOG2_SIZE;
        let fract_scale = 1 << fract_bits;
        let fract_mask = fract_scale - 1;

        let significand = ctx.b.build_int_truncate(
            ctx.b.build_right_shift(
                ctx.b.build_left_shift(x_int, exponent, ""),
                get_i64_spread(ctx.context, 20),
                false,
                "",
            ),
            ctx.context.i32_type().vec_type(2),
            "significand",
        );

        let a_index = ctx.b.build_right_shift(
            significand,
            get_i32_spread(ctx.context, fract_bits as u64),
            false,
            "a.index",
        );
        let fract = ctx.b.build_and(
            significand,
            get_i32_spread(ctx.context, fract_mask as u64),
            "fract",
        );

        let left_index = ctx.context.i32_type().const_int(0, false);
        let right_index = ctx.context.i32_type().const_int(1, false);

        let a_ptr = unsafe {
            ctx.b.build_gep(
                &sine_table_ptr,
                &[get_i32_spread(ctx.context, 0), a_index],
                "a.ptr",
            )
        };

        let a_left_ptr = ctx
            .b
            .build_extract_element(&a_ptr, &left_index, "a.left.ptr")
            .into_pointer_value();
        let a_right_ptr = ctx
            .b
            .build_extract_element(&a_ptr, &right_index, "a.right.ptr")
            .into_pointer_value();

        let a_left_val = ctx.b.build_load(&a_left_ptr, "a.left").into_float_value();
        let a_right_val = ctx.b.build_load(&a_right_ptr, "a.right").into_float_value();
        let a_val = ctx
            .b
            .build_insert_element(
                &ctx.b
                    .build_insert_element(
                        &ctx.context.f64_type().vec_type(2).get_undef(),
                        &a_left_val,
                        &left_index,
                        "",
                    ).into_vector_value(),
                &a_right_val,
                &right_index,
                "a",
            ).into_vector_value();

        let b_index = ctx
            .b
            .build_int_add(a_index, get_i32_spread(ctx.context, 1), "b.index");
        let b_ptr = unsafe {
            ctx.b.build_gep(
                &sine_table_ptr,
                &[get_i32_spread(ctx.context, 0), b_index],
                "b.ptr",
            )
        };

        let b_left_ptr = ctx
            .b
            .build_extract_element(&b_ptr, &left_index, "b.left.ptr")
            .into_pointer_value();
        let b_right_ptr = ctx
            .b
            .build_extract_element(&b_ptr, &right_index, "b.right.ptr")
            .into_pointer_value();

        let b_left_val = ctx.b.build_load(&b_left_ptr, "b.left").into_float_value();
        let b_right_val = ctx.b.build_load(&b_right_ptr, "b.right").into_float_value();
        let b_val = ctx
            .b
            .build_insert_element(
                &ctx.b
                    .build_insert_element(
                        &ctx.context.f64_type().vec_type(2).get_undef(),
                        &b_left_val,
                        &left_index,
                        "",
                    ).into_vector_value(),
                &b_right_val,
                &right_index,
                "b",
            ).into_vector_value();

        let fract_double = ctx.b.build_signed_int_to_float(
            fract,
            ctx.context.f64_type().vec_type(2),
            "fract.float",
        );
        let fract_mix = ctx.b.build_float_mul(
            fract_double,
            util::get_vec_spread(ctx.context, 1. / fract_scale as f64),
            "fract.mix",
        );

        let result_vec = ctx.b.build_float_add(
            a_val,
            ctx.b
                .build_float_mul(ctx.b.build_float_sub(b_val, a_val, ""), fract_mix, ""),
            "",
        );
        ctx.b.build_return(Some(&result_vec));
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

fn build_cos_v2f64(module: &Module) {
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

fn build_mod_v2f64(module: &Module) {
    // todo
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

fn build_pow_v2f64(module: &Module) {
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

fn build_exp_v2f64(module: &Module) {
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

fn build_exp2_v2f64(module: &Module) {
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

fn build_exp10_v2f64(module: &Module) {
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

fn build_log_v2f64(module: &Module) {
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

fn build_log2_v2f64(module: &Module) {
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

fn build_log10_v2f64(module: &Module) {
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

fn build_asin_v2f64(module: &Module) {
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

fn build_acos_v2f64(module: &Module) {
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

fn build_atan_v2f64(module: &Module) {
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

fn build_atan2_v2f64(module: &Module) {
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

fn build_sinh_v2f64(module: &Module) {
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

fn build_cosh_v2f64(module: &Module) {
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

fn build_tanh_v2f64(module: &Module) {
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

fn build_hypot_v2f64(module: &Module) {
    // todo
}
