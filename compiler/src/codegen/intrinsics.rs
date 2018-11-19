use codegen::{build_context_function, util, BuilderContext, TargetProperties};
use inkwell::attribute::AttrKind;
use inkwell::module::{Linkage, Module};
use inkwell::targets::TargetData;
use inkwell::types::{BasicType, VectorType};
use inkwell::values::FunctionValue;
use inkwell::{AddressSpace, FloatPredicate, IntPredicate};

pub fn memcpy(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.memcpy.p0i8.p0i8.i64", false, &|| {
        let context = module.get_context();
        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(
                &[
                    &context.i8_type().ptr_type(AddressSpace::Generic),
                    &context.i8_type().ptr_type(AddressSpace::Generic),
                    &context.i64_type(),
                    &context.i32_type(),
                    &context.bool_type(),
                ],
                false,
            ),
        )
    })
}

pub fn realloc(module: &Module, target: &TargetData) -> FunctionValue {
    util::get_or_create_func(module, "realloc", false, &|| {
        let ptr_type = module
            .get_context()
            .i8_type()
            .ptr_type(AddressSpace::Generic);
        (
            Linkage::ExternalLinkage,
            ptr_type.fn_type(
                &[
                    &ptr_type,
                    &target.int_ptr_type_in_context(&module.get_context()),
                ],
                false,
            ),
        )
    })
}

pub fn memset(module: &Module, target: &TargetData) -> FunctionValue {
    let target_ptr_type = target.int_ptr_type_in_context(&module.get_context());
    let intrinsic_name = format!("llvm.memset.p0i8.i{}", target_ptr_type.get_bit_width());
    util::get_or_create_func(module, &intrinsic_name, false, &|| {
        let ptr_type = module
            .get_context()
            .i8_type()
            .ptr_type(AddressSpace::Generic);
        (
            Linkage::ExternalLinkage,
            module.get_context().void_type().fn_type(
                &[
                    &ptr_type,
                    &module.get_context().i8_type(),
                    &target_ptr_type,
                    &module.get_context().i32_type(),
                    &module.get_context().bool_type(),
                ],
                false,
            ),
        )
    })
}

pub fn pow_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.pow.v2f64", false, &|| {
        let context = module.get_context();
        let v2f64_type = context.f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

pub fn pow_f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.pow.f64", false, &|| {
        let context = module.get_context();
        let f64_type = context.f64_type();
        (
            Linkage::ExternalLinkage,
            f64_type.fn_type(&[&f64_type, &f64_type], false),
        )
    })
}

pub fn exp_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.exp.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn exp_f64(module: &Module) -> FunctionValue {
    get_float_asm_func(module, "fldl2e; fmulp; fld %st; frndint; fsubr %st, %st(1); fxch; f2xm1; fld1; faddp; fscale; fstp %st(1)", "={st},0,~{dirflag},~{fpsr},~{flags}")
}

pub fn log_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.log.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn log_f64(module: &Module) -> FunctionValue {
    get_float_asm_func(
        module,
        "fldln2; fxch; fyl2x",
        "={st},0,~{dirflag},~{fpsr},~{flags}",
    )
}

pub fn log10_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.log10.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn log10_f64(module: &Module) -> FunctionValue {
    get_float_asm_func(
        module,
        "fldlg2; fxch; fyl2x",
        "={st},0,~{dirflag},~{fpsr},~{flags}",
    )
}

pub fn log2_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.log2.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn log2_f64(module: &Module) -> FunctionValue {
    get_float_asm_func(
        module,
        "fld1; fxch; fyl2x",
        "={st},0,~{dirflag},~{fpsr},~{flags}",
    )
}

pub fn cos_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.cos.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn cos_f64(module: &Module) -> FunctionValue {
    get_float_asm_func(module, "fcos", "={st},0,~{dirflag},~{fpsr},~{flags}")
}

pub fn sin_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.sin.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn sin_f64(module: &Module) -> FunctionValue {
    get_float_asm_func(module, "fsin", "={st},0,~{dirflag},~{fpsr},~{flags}")
}

pub fn tan_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.tan.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn tan_f64(module: &Module) -> FunctionValue {
    get_float_asm_func(
        module,
        "fptan; fstp $0",
        "={st},0,~{dirflag},~{fpsr},~{flags}",
    )
}

pub fn sqrt_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.sqrt.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn ceil_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.ceil.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn floor_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.floor.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn fabs_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.fabs.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn minnum_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.minnum.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

pub fn minnum_f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.minnum.f64", false, &|| {
        let f64_type = module.get_context().f64_type();
        (
            Linkage::ExternalLinkage,
            f64_type.fn_type(&[&f64_type, &f64_type], false),
        )
    })
}

pub fn maxnum_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.maxnum.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

pub fn maxnum_f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.maxnum.f64", false, &|| {
        let f64_type = module.get_context().f64_type();
        (
            Linkage::ExternalLinkage,
            f64_type.fn_type(&[&f64_type, &f64_type], false),
        )
    })
}

pub fn ctlz_i64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.ctlz.i64", false, &|| {
        let i64_type = module.get_context().i64_type();
        (
            Linkage::ExternalLinkage,
            i64_type.fn_type(&[&i64_type, &module.get_context().bool_type()], false),
        )
    })
}

pub fn copysign_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.copysign.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f64_type.fn_type(&[&v2f64_type, &v2f64_type], false),
        )
    })
}

pub fn eucrem_v2i32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.eucrem.v2i32", false, &|| {
        let v2i32_type = module.get_context().i32_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2i32_type.fn_type(&[&v2i32_type, &v2i32_type], false),
        )
    })
}

pub fn fract_v2f64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.fract.v2f64", false, &|| {
        let v2f64_type = module.get_context().f64_type().vec_type(2);
        (
            Linkage::PrivateLinkage,
            v2f64_type.fn_type(&[&v2f64_type], false),
        )
    })
}

pub fn next_power_i64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.nextpower.i64", false, &|| {
        let i64_type = module.get_context().i64_type();
        (
            Linkage::PrivateLinkage,
            i64_type.fn_type(&[&i64_type], false),
        )
    })
}

pub fn profile_timestamp_i64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "profile_timestamp", true, &|| {
        let i64_type = module.get_context().i64_type();
        (Linkage::ExternalLinkage, i64_type.fn_type(&[], false))
    })
}

pub fn build_intrinsics(module: &Module, target: &TargetProperties) {
    //build_pow_v2f64(module, target);
    //build_pow_f64(module, target);
    build_exp_v2f64(module, target);
    build_log_v2f64(module, target);
    build_log10_v2f64(module, target);
    build_log2_v2f64(module, target);
    build_sin_v2f64(module, target);
    build_cos_v2f64(module, target);
    build_tan_v2f64(module, target);
    build_eucrem_v2i32(module, target);
    build_fract_v2f64(module, target);
    build_next_power_i64(module, target);
}

fn build_float_splat_function(
    module: &Module,
    target: &TargetProperties,
    gen_func: FunctionValue,
    target_func: FunctionValue,
) {
    build_context_function(module, gen_func, target, &|ctx: BuilderContext| {
        let index1 = ctx.context.i32_type().const_int(0, false);
        let index2 = ctx.context.i32_type().const_int(1, false);

        let x_val = ctx.func.get_nth_param(0).unwrap().into_vector_value();
        let x1_val = ctx
            .b
            .build_extract_element(&x_val, &index1, "x.1")
            .into_float_value();
        let sin1_val = ctx
            .b
            .build_call(&target_func, &[&x1_val], "out.1", false)
            .left()
            .unwrap()
            .into_float_value();
        let x2_val = ctx
            .b
            .build_extract_element(&x_val, &index2, "x.2")
            .into_float_value();
        let sin2_val = ctx
            .b
            .build_call(&target_func, &[&x2_val], "out.2", false)
            .left()
            .unwrap()
            .into_float_value();

        let vec = ctx
            .b
            .build_insert_element(
                &ctx.b
                    .build_insert_element(
                        &ctx.context.f64_type().vec_type(2).get_undef(),
                        &sin1_val,
                        &index1,
                        "out.combine1",
                    ).into_vector_value(),
                &sin2_val,
                &index2,
                "out.combine2",
            ).into_vector_value();

        ctx.b.build_return(Some(&vec));
    })
}

fn get_float_asm_func(module: &Module, asm: &str, constraints: &str) -> FunctionValue {
    let context = module.get_context();
    let fn_type = context.f64_type().fn_type(&[&context.f64_type()], false);
    let asm_val = fn_type.as_asm(asm, constraints, false, false);
    //asm_val.add_attribute(context.get_enum_attr(AttrKind::NoUnwind, 1));
    //asm_val.add_attribute(context.get_enum_attr(AttrKind::ReadNone, 1));
    asm_val
}

fn build_pow_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(module, pow_v2f64(module), target, &|ctx: BuilderContext| {
        let exp_intrinsic = exp_v2f64(module);
        let log_intrinsic = log_v2f64(module);
        let abs_intrinsic = fabs_v2f64(module);
        let floor_intrinsic = floor_v2f64(module);
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
            ).into_vector_value();
        let r = ctx
            .b
            .build_call(
                &exp_intrinsic,
                &[&ctx.b.build_float_mul(
                    ctx.b
                        .build_call(&log_intrinsic, &[&norm_x], "", false)
                        .left()
                        .unwrap()
                        .into_vector_value(),
                    y_vec,
                    "",
                )],
                "r",
                false,
            ).left()
            .unwrap()
            .into_vector_value();

        // the result is negative if x < 0 and if y is integer and odd
        let abs_y = ctx
            .b
            .build_call(&abs_intrinsic, &[&y_vec], "", false)
            .left()
            .unwrap()
            .into_vector_value();
        let mod_y = ctx.b.build_float_sub(
            abs_y,
            ctx.b.build_float_mul(
                util::get_vec_spread(ctx.context, 2.),
                ctx.b
                    .build_call(
                        &floor_intrinsic,
                        &[&ctx.b.build_float_mul(
                            abs_y,
                            util::get_vec_spread(ctx.context, 0.5),
                            "",
                        )],
                        "",
                        false,
                    ).left()
                    .unwrap()
                    .into_vector_value(),
                "",
            ),
            "mody",
        );
        let sign = ctx.b.build_float_add(
            ctx.b
                .build_call(&copysign_intrinsic, &[&mod_y, &norm_x], "", false)
                .left()
                .unwrap()
                .into_vector_value(),
            util::get_vec_spread(ctx.context, 0.5),
            "sign",
        );
        let norm_r = ctx
            .b
            .build_call(&copysign_intrinsic, &[&r, &sign], "normr", false)
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
            ).into_vector_value();
        ctx.b.build_return(Some(&res));
    });
}

fn build_pow_f64(module: &Module, target: &TargetProperties) {
    build_context_function(module, pow_f64(module), target, &|ctx: BuilderContext| {
        let pow_intrinsic = pow_v2f64(module);

        let x_float = ctx.func.get_nth_param(0).unwrap().into_float_value();
        let y_float = ctx.func.get_nth_param(1).unwrap().into_float_value();

        // put the values into undefined vectors to call the vectorized pow
        let index_0 = ctx.context.i32_type().const_int(0, false);
        let x_vec = ctx
            .b
            .build_insert_element(
                &ctx.context.f64_type().vec_type(2).get_undef(),
                &x_float,
                &index_0,
                "xvec",
            ).into_vector_value();
        let y_vec = ctx
            .b
            .build_insert_element(
                &ctx.context.f64_type().vec_type(2).get_undef(),
                &y_float,
                &index_0,
                "yvec",
            ).into_vector_value();

        let res_vec = ctx
            .b
            .build_call(&pow_intrinsic, &[&x_vec, &y_vec], "res.vec", false)
            .left()
            .unwrap()
            .into_vector_value();
        let res_float = ctx
            .b
            .build_extract_element(&res_vec, &index_0, "res.float")
            .into_float_value();

        ctx.b.build_return(Some(&res_float));
    });
}

fn build_exp_v2f64(module: &Module, target: &TargetProperties) {
    build_float_splat_function(module, target, exp_v2f64(module), exp_f64(module));
}

fn build_log_v2f64(module: &Module, target: &TargetProperties) {
    build_float_splat_function(module, target, log_v2f64(module), log_f64(module));
}

fn build_log10_v2f64(module: &Module, target: &TargetProperties) {
    build_float_splat_function(module, target, log10_v2f64(module), log10_f64(module));
}

fn build_log2_v2f64(module: &Module, target: &TargetProperties) {
    build_float_splat_function(module, target, log2_v2f64(module), log2_f64(module));
}

fn build_sin_v2f64(module: &Module, target: &TargetProperties) {
    build_float_splat_function(module, target, sin_v2f64(module), sin_f64(module));
}

fn build_cos_v2f64(module: &Module, target: &TargetProperties) {
    build_float_splat_function(module, target, cos_v2f64(module), cos_f64(module));
}

fn build_tan_v2f64(module: &Module, target: &TargetProperties) {
    build_float_splat_function(module, target, tan_v2f64(module), tan_f64(module));
}

fn build_eucrem_v2i32(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        eucrem_v2i32(module),
        target,
        &|ctx: BuilderContext| {
            let x_val = ctx.func.get_nth_param(0).unwrap().into_vector_value();
            let y_val = ctx.func.get_nth_param(1).unwrap().into_vector_value();

            // if we assume y is always positive (which we do here), this is equal to
            // x % y when x is positive, otherwise x % y + y
            let rem_val = ctx.b.build_int_signed_rem(x_val, y_val, "rem");
            let const_zero = VectorType::const_vector(&[
                &ctx.context.i32_type().const_int(0, false),
                &ctx.context.i32_type().const_int(0, false),
            ]);
            let lt_zero = ctx
                .b
                .build_int_compare(IntPredicate::SLT, x_val, const_zero, "");
            let shift_amt = ctx.b.build_int_nuw_mul(
                y_val,
                ctx.b.build_int_z_extend(lt_zero, y_val.get_type(), ""),
                "",
            );

            let result_val = ctx.b.build_int_nsw_add(rem_val, shift_amt, "");
            ctx.b.build_return(Some(&result_val));
        },
    );
}

fn build_fract_v2f64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        fract_v2f64(module),
        target,
        &|ctx: BuilderContext| {
            let floor_intrinsic = floor_v2f64(ctx.module);
            let x_val = ctx.func.get_nth_param(0).unwrap().into_vector_value();

            let x_floored = ctx
                .b
                .build_call(&floor_intrinsic, &[&x_val], "xfloored", false)
                .left()
                .unwrap()
                .into_vector_value();

            let fract_val = ctx.b.build_float_sub(x_val, x_floored, "xfract");

            ctx.b.build_return(Some(&fract_val));
        },
    )
}

fn build_next_power_i64(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        next_power_i64(module),
        target,
        &|ctx: BuilderContext| {
            let ctlz_intrinsic = ctlz_i64(ctx.module);

            let zero_true_block = ctx.context.append_basic_block(&ctx.func, "zero.true");
            let zero_false_block = ctx.context.append_basic_block(&ctx.func, "zero.false");

            let in_val = ctx.func.get_nth_param(0).unwrap().into_int_value();

            // we need special behavior for 0 since this results in 64 << 64, undefined behavior in LLVM
            let is_zero = ctx.b.build_int_compare(
                IntPredicate::EQ,
                in_val,
                ctx.context.i64_type().const_int(0, false),
                "zero",
            );
            ctx.b
                .build_conditional_branch(&is_zero, &zero_true_block, &zero_false_block);

            ctx.b.position_at_end(&zero_true_block);
            ctx.b
                .build_return(Some(&ctx.context.i64_type().const_int(0, false)));

            ctx.b.position_at_end(&zero_false_block);
            let next_pow_val = ctx.b.build_left_shift(
                ctx.context.i64_type().const_int(1, false),
                ctx.b.build_int_nuw_sub(
                    ctx.context.i64_type().const_int(64, false),
                    ctx.b
                        .build_call(
                            &ctlz_intrinsic,
                            &[
                                &ctx.b.build_int_nuw_sub(
                                    in_val,
                                    ctx.context.i64_type().const_int(1, false),
                                    "",
                                ),
                                &ctx.context.bool_type().const_int(0, false),
                            ],
                            "",
                            false,
                        ).left()
                        .unwrap()
                        .into_int_value(),
                    "",
                ),
                "result",
            );
            ctx.b.build_return(Some(&next_pow_val));
        },
    );
}
