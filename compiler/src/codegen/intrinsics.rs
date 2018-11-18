use codegen::util;
use inkwell::module::{Linkage, Module};
use inkwell::targets::TargetData;
use inkwell::types::{BasicType, VectorType};
use inkwell::values::FunctionValue;
use inkwell::{AddressSpace, IntPredicate};

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

pub fn pow_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.pow.v2f32", false, &|| {
        let context = module.get_context();
        let v2f32_type = context.f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type, &v2f32_type], false),
        )
    })
}

pub fn pow_f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.pow.f32", false, &|| {
        let context = module.get_context();
        let f32_type = context.f32_type();
        (
            Linkage::ExternalLinkage,
            f32_type.fn_type(&[&f32_type, &f32_type], false),
        )
    })
}

pub fn exp_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.exp.v2f32", false, &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn exp2_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.exp2.v2f32", false, &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn log_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.log.v2f32", false, &|| {
        let context = module.get_context();
        let v2f32_type = context.f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn log10_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.log10.v2f32", false, &|| {
        let context = module.get_context();
        let v2f32_type = context.f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn log2_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.log2.v2f32", false, &|| {
        let context = module.get_context();
        let v2f32_type = context.f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn cos_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.cos.v2f32", false, &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn cos_f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.cos.f32", false, &|| {
        let f32_type = module.get_context().f32_type();
        (
            Linkage::ExternalLinkage,
            f32_type.fn_type(&[&f32_type], false),
        )
    })
}

pub fn sin_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.sin.v2f32", false, &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn sin_f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.sin.f32", false, &|| {
        let f32_type = module.get_context().f32_type();
        (
            Linkage::ExternalLinkage,
            f32_type.fn_type(&[&f32_type], false),
        )
    })
}

pub fn sqrt_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.sqrt.v2f32", false, &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn ceil_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.ceil.v2f32", false, &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn floor_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.floor.v2f32", false, &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn fabs_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.fabs.v2f32", false, &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type], false),
        )
    })
}

pub fn minnum_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.minnum.v2f32", false, &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type, &v2f32_type], false),
        )
    })
}

pub fn minnum_f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.minnum.f32", false, &|| {
        let f32_type = module.get_context().f32_type();
        (
            Linkage::ExternalLinkage,
            f32_type.fn_type(&[&f32_type, &f32_type], false),
        )
    })
}

pub fn maxnum_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.maxnum.v2f32", false, &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type, &v2f32_type], false),
        )
    })
}

pub fn maxnum_f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.maxnum.f32", false, &|| {
        let f32_type = module.get_context().f32_type();
        (
            Linkage::ExternalLinkage,
            f32_type.fn_type(&[&f32_type, &f32_type], false),
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

pub fn copysign_v2f32(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "llvm.copysign.v2f32", false, &|| {
        let v2f32_type = module.get_context().f32_type().vec_type(2);
        (
            Linkage::ExternalLinkage,
            v2f32_type.fn_type(&[&v2f32_type, &v2f32_type], false),
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

pub fn next_power_i64(module: &Module) -> FunctionValue {
    util::get_or_create_func(module, "maxim.nextpower.i64", true, &|| {
        let i64_type = module.get_context().i64_type();
        (
            Linkage::ExternalLinkage,
            i64_type.fn_type(&[&i64_type], false),
        )
    })
}

pub fn build_intrinsics(module: &Module) {
    build_eucrem_v2i32(module);
    build_next_power_i64(module);
}

fn build_eucrem_v2i32(module: &Module) {
    let func = eucrem_v2i32(module);
    let context = module.get_context();
    let entry_block = context.append_basic_block(&func, "entry");
    let builder = context.create_builder();
    builder.set_fast_math_all();
    builder.position_at_end(&entry_block);

    let x_val = func.get_nth_param(0).unwrap().into_vector_value();
    let y_val = func.get_nth_param(1).unwrap().into_vector_value();

    // if we assume y is always positive (which we do here), this is equal to
    // x % y when x is positive, otherwise x % y + y
    let rem_val = builder.build_int_signed_rem(x_val, y_val, "rem");
    let const_zero = VectorType::const_vector(&[
        &context.i32_type().const_int(0, false),
        &context.i32_type().const_int(0, false),
    ]);
    let lt_zero = builder.build_int_compare(IntPredicate::SLT, x_val, const_zero, "");
    let shift_amt = builder.build_int_mul(
        y_val,
        builder.build_int_z_extend(lt_zero, y_val.get_type(), ""),
        "",
    );

    let result_val = builder.build_int_add(rem_val, shift_amt, "");
    builder.build_return(Some(&result_val));
}

fn build_next_power_i64(module: &Module) {
    let ctlz_intrinsic = ctlz_i64(module);

    let func = next_power_i64(module);
    let context = module.get_context();
    let entry_block = context.append_basic_block(&func, "entry");
    let zero_true_block = context.append_basic_block(&func, "zero.true");
    let zero_false_block = context.append_basic_block(&func, "zero.false");
    let builder = context.create_builder();
    builder.set_fast_math_all();
    builder.position_at_end(&entry_block);

    let in_val = func.get_nth_param(0).unwrap().into_int_value();

    // we need special behavior for 0 since this results in 64 << 64, undefined behavior in LLVM
    let is_zero = builder.build_int_compare(
        IntPredicate::EQ,
        in_val,
        context.i64_type().const_int(0, false),
        "zero",
    );
    builder.build_conditional_branch(&is_zero, &zero_true_block, &zero_false_block);

    builder.position_at_end(&zero_true_block);
    builder.build_return(Some(&context.i64_type().const_int(0, false)));

    builder.position_at_end(&zero_false_block);
    let next_pow_val = builder.build_left_shift(
        context.i64_type().const_int(1, false),
        builder.build_int_sub(
            context.i64_type().const_int(64, false),
            builder
                .build_call(
                    &ctlz_intrinsic,
                    &[
                        &builder.build_int_sub(in_val, context.i64_type().const_int(1, false), ""),
                        &context.bool_type().const_int(0, false),
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
    builder.build_return(Some(&next_pow_val));
}
