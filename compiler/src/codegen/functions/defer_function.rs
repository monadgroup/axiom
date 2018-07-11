use super::{Function, FunctionContext, VarArgs};
use ast::FormType;
use codegen::values::NumValue;
use codegen::{globals, intrinsics, util, BuilderContext};
use inkwell::context::Context;
use inkwell::types::StructType;
use inkwell::values::PointerValue;
use inkwell::{FloatPredicate, IntPredicate};
use mir::block;

pub struct NextFunction {}
impl Function for NextFunction {
    fn function_type() -> block::Function {
        block::Function::Next
    }

    fn data_type(context: &Context) -> StructType {
        NumValue::get_type(context)
    }

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        _varargs: Option<VarArgs>,
        result: PointerValue,
    ) {
        let last_num = NumValue::new(func.data_ptr);
        let new_num = NumValue::new(args[0]);
        let result_num = NumValue::new(result);

        last_num.copy_to(func.ctx.b, func.ctx.module, &result_num);
        new_num.copy_to(func.ctx.b, func.ctx.module, &last_num);
    }
}

pub struct AmplitudeFunction {}
impl Function for AmplitudeFunction {
    fn function_type() -> block::Function {
        block::Function::Amplitude
    }

    fn data_type(context: &Context) -> StructType {
        context.struct_type(&[&context.f32_type().vec_type(2)], false)
    }

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        _varargs: Option<VarArgs>,
        result: PointerValue,
    ) {
        let abs_intrinsic = intrinsics::fabs_v2f32(func.ctx.module);
        let exp_intrinsic = intrinsics::exp_v2f32(func.ctx.module);

        let current_estimate_ptr = unsafe {
            func.ctx
                .b
                .build_struct_gep(&func.data_ptr, 0, "currentestimate.ptr")
        };

        let param_num = NumValue::new(args[0]);
        let result_num = NumValue::new(result);

        let param_vec = param_num.get_vec(func.ctx.b);
        let abs_input = func.ctx
            .b
            .build_call(&abs_intrinsic, &[&param_vec], "inputabs", false)
            .left()
            .unwrap()
            .into_vector_value();
        let current_estimate = func.ctx
            .b
            .build_load(&current_estimate_ptr, "currentestimate")
            .into_vector_value();

        // b0 = 1 - exp(-1 / (0.05f * sampleRate))
        let b0 = func.ctx.b.build_float_sub(
            util::get_vec_spread(func.ctx.context, 1.),
            func.ctx
                .b
                .build_call(
                    &exp_intrinsic,
                    &[&func.ctx.b.build_float_div(
                        util::get_vec_spread(func.ctx.context, -1.),
                        func.ctx.b.build_float_mul(
                            util::get_vec_spread(func.ctx.context, 0.05),
                            func.ctx
                                .b
                                .build_load(
                                    &globals::get_sample_rate(func.ctx.module).as_pointer_value(),
                                    "samplerate",
                                )
                                .into_vector_value(),
                            "",
                        ),
                        "",
                    )],
                    "",
                    false,
                )
                .left()
                .unwrap()
                .into_vector_value(),
            "",
        );
        let new_estimate = func.ctx.b.build_float_add(
            current_estimate,
            func.ctx.b.build_float_mul(
                b0,
                func.ctx.b.build_float_sub(abs_input, current_estimate, ""),
                "",
            ),
            "newestimate",
        );
        func.ctx.b.build_store(&current_estimate_ptr, &new_estimate);

        result_num.set_vec(func.ctx.b, &new_estimate);
        result_num.set_form(
            func.ctx.b,
            &func.ctx
                .context
                .i8_type()
                .const_int(FormType::Amplitude as u64, false),
        );
    }
}

pub struct HoldFunction {}
impl Function for HoldFunction {
    fn function_type() -> block::Function {
        block::Function::Hold
    }

    fn data_type(context: &Context) -> StructType {
        context.struct_type(
            &[
                &context.f32_type().vec_type(2),  // stored value
                &context.bool_type().vec_type(2), // last gate value
            ],
            false,
        )
    }

    fn gen_real_args(ctx: &mut BuilderContext, mut args: Vec<PointerValue>) -> Vec<PointerValue> {
        if args.len() < 3 {
            let mut else_constant = NumValue::new_undef(ctx.context, ctx.allocb);
            else_constant.store(ctx.b, &NumValue::get_const(ctx.context, 0., 0., 0));
            args.push(else_constant.val);
        }
        args
    }

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        _varargs: Option<VarArgs>,
        result: PointerValue,
    ) {
        let val_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 0, "val.ptr") };
        let gate_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 1, "gate.ptr") };

        let x_num = NumValue::new(args[0]);
        let gate_num = NumValue::new(args[1]);
        let else_num = NumValue::new(args[2]);
        let result_num = NumValue::new(result);

        let gate_vec = gate_num.get_vec(func.ctx.b);
        let gate_bool = func.ctx.b.build_float_compare(
            FloatPredicate::ONE,
            gate_vec,
            util::get_vec_spread(func.ctx.context, 0.),
            "gatebool",
        );

        let last_gate = func.ctx
            .b
            .build_load(&gate_ptr, "lastgate")
            .into_vector_value();
        func.ctx.b.build_store(&gate_ptr, &gate_bool);

        let is_rising =
            func.ctx
                .b
                .build_int_compare(IntPredicate::UGT, gate_bool, last_gate, "isrising");

        let current_vec = x_num.get_vec(func.ctx.b);
        let last_vec = func.ctx
            .b
            .build_load(&val_ptr, "lastval")
            .into_vector_value();
        let new_value = func.ctx
            .b
            .build_select(is_rising, current_vec, last_vec, "newval")
            .into_vector_value();
        func.ctx.b.build_store(&val_ptr, &new_value);

        let else_vec = else_num.get_vec(func.ctx.b);
        let result_vec = func.ctx
            .b
            .build_select(gate_bool, new_value, else_vec, "resultvec")
            .into_vector_value();

        let x_form = x_num.get_form(func.ctx.b);
        result_num.set_form(func.ctx.b, &x_form);
        result_num.set_vec(func.ctx.b, &result_vec);
    }
}

pub struct AccumFunction {}
impl Function for AccumFunction {
    fn function_type() -> block::Function {
        block::Function::Accum
    }

    fn data_type(context: &Context) -> StructType {
        context.struct_type(&[&context.f32_type().vec_type(2)], false)
    }

    fn gen_real_args(ctx: &mut BuilderContext, mut args: Vec<PointerValue>) -> Vec<PointerValue> {
        if args.len() < 3 {
            let mut base_constant = NumValue::new_undef(ctx.context, ctx.allocb);
            base_constant.store(ctx.b, &NumValue::get_const(ctx.context, 0., 0., 0));
            args.push(base_constant.val);
        }
        args
    }

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        _varargs: Option<VarArgs>,
        result: PointerValue,
    ) {
        let accum_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 0, "accum.ptr") };

        let x_num = NumValue::new(args[0]);
        let gate_num = NumValue::new(args[1]);
        let base_num = NumValue::new(args[2]);
        let result_num = NumValue::new(result);

        let accum_vec = func.ctx
            .b
            .build_load(&accum_ptr, "accum")
            .into_vector_value();
        let x_vec = x_num.get_vec(func.ctx.b);
        let incremented_vec = func.ctx.b.build_float_add(accum_vec, x_vec, "accum.incr");

        let gate_vec = gate_num.get_vec(func.ctx.b);
        let gate_bool = func.ctx.b.build_float_compare(
            FloatPredicate::ONE,
            gate_vec,
            util::get_vec_spread(func.ctx.context, 0.),
            "gatebool",
        );

        let base_vec = base_num.get_vec(func.ctx.b);
        let new_accum = func.ctx
            .b
            .build_select(gate_bool, incremented_vec, base_vec, "newaccum")
            .into_vector_value();
        func.ctx.b.build_store(&accum_ptr, &new_accum);

        let x_form = x_num.get_form(func.ctx.b);
        result_num.set_form(func.ctx.b, &x_form);
        result_num.set_vec(func.ctx.b, &new_accum);
    }
}
