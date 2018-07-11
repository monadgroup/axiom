use super::{Function, FunctionContext, VarArgs};
use ast::FormType;
use codegen::values::NumValue;
use codegen::{globals, intrinsics, util};
use inkwell::context::Context;
use inkwell::types::StructType;
use inkwell::values::PointerValue;
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

/*pub struct HoldFunction {}
impl Function for HoldFunction {

}

pub struct AccumFunction {}
impl Function for AccumFunction {

}*/
