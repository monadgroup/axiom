use codegen::{build_context_function, util, BuilderContext, TargetProperties};
use inkwell::module::{Linkage, Module};
use inkwell::targets::TargetData;
use inkwell::types::{BasicType, VectorType};
use inkwell::values::FunctionValue;
use inkwell::{AddressSpace, IntPredicate};

pub fn memcpy(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.memcpy.p0i8.p0i8.i64", true, &|| {
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
    util::get_or_create_func(module, &intrinsic_name, true, &|| {
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

pub fn ctlz_i32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.ctlz.i32", true, &|| {
        let i32_type = module.get_context().i32_type();
        (
            Linkage::ExternalLinkage,
            i32_type.fn_type(&[&i32_type, &module.get_context().bool_type()], false),
        )
    })
}

pub fn eucrem_v2i32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.eucrem.v2i32", true, &|| {
        let v2i32_type = module.get_context().i32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2i32_type.fn_type(&[&v2i32_type, &v2i32_type], false),
        )
    })
}

pub fn next_power_i32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.nextpower.i32", true, &|| {
        let i32_type = module.get_context().i32_type();
        (
            Linkage::PrivateLinkage,
            i32_type.fn_type(&[&i32_type], false),
        )
    })
}

pub fn profile_timestamp_i64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "profile_timestamp", false, &|| {
        let i64_type = module.get_context().i64_type();
        (Linkage::ExternalLinkage, i64_type.fn_type(&[], false))
    })
}

pub fn build_intrinsics(module: &Module, target: &TargetProperties) {
    build_eucrem_v2i32(module, target);
    build_next_power_i32(module, target);
}

// Integer modulo (euclidian remainder)
// This means the function returns a positive value when X is negative, as opposed to LLVM's
// 'srem' instruction which is negative when X is negative.
fn build_eucrem_v2i32(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        eucrem_v2i32(module),
        target,
        &|ctx: BuilderContext| {
            let x_val = ctx.func.get_nth_param(0).unwrap().into_vector_value();
            let y_val = ctx.func.get_nth_param(1).unwrap().into_vector_value();

            // note: we need to to the rem operation on each side separately, otherwise llvm on x86
            // generates calls to an internal math function (which we don't want to depend on).
            let left_index = ctx.context.i32_type().const_int(0, false);
            let right_index = ctx.context.i32_type().const_int(1, false);
            let left_rem_val = ctx.b.build_int_signed_rem(
                ctx.b
                    .build_extract_element(&x_val, &left_index, "")
                    .into_int_value(),
                ctx.b
                    .build_extract_element(&y_val, &left_index, "")
                    .into_int_value(),
                "",
            );
            let right_rem_val = ctx.b.build_int_signed_rem(
                ctx.b
                    .build_extract_element(&x_val, &right_index, "")
                    .into_int_value(),
                ctx.b
                    .build_extract_element(&y_val, &right_index, "")
                    .into_int_value(),
                "",
            );

            let rem_val = ctx
                .b
                .build_insert_element(
                    &ctx.b
                        .build_insert_element(
                            &ctx.context.i32_type().vec_type(2).get_undef(),
                            &left_rem_val,
                            &left_index,
                            "",
                        ).into_vector_value(),
                    &right_rem_val,
                    &right_index,
                    "",
                ).into_vector_value();

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

fn build_next_power_i32(module: &Module, target: &TargetProperties) {
    build_context_function(
        module,
        next_power_i32(module),
        target,
        &|ctx: BuilderContext| {
            let ctlz_intrinsic = ctlz_i32(ctx.module);

            let zero_true_block = ctx.context.append_basic_block(&ctx.func, "zero.true");
            let zero_false_block = ctx.context.append_basic_block(&ctx.func, "zero.false");

            let in_val = ctx.func.get_nth_param(0).unwrap().into_int_value();

            // we need special behavior for 0 since this results in 32 << 32, undefined behavior in LLVM
            let is_zero = ctx.b.build_int_compare(
                IntPredicate::EQ,
                in_val,
                ctx.context.i32_type().const_int(0, false),
                "zero",
            );
            ctx.b
                .build_conditional_branch(&is_zero, &zero_true_block, &zero_false_block);

            ctx.b.position_at_end(&zero_true_block);
            ctx.b
                .build_return(Some(&ctx.context.i32_type().const_int(0, false)));

            ctx.b.position_at_end(&zero_false_block);
            let next_pow_val = ctx.b.build_left_shift(
                ctx.context.i32_type().const_int(1, false),
                ctx.b.build_int_nuw_sub(
                    ctx.context.i32_type().const_int(32, false),
                    ctx.b
                        .build_call(
                            &ctlz_intrinsic,
                            &[
                                &ctx.b.build_int_nuw_sub(
                                    in_val,
                                    ctx.context.i32_type().const_int(1, false),
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
