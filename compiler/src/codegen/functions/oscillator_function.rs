use super::{Function, FunctionContext, VarArgs};
use ast::FormType;
use codegen::values::NumValue;
use codegen::{globals, intrinsics, util, BuilderContext};
use inkwell::context::Context;
use inkwell::types::StructType;
use inkwell::values::{PointerValue, VectorValue};
use mir::block;
use std::f32::consts;

fn gen_periodic_real_args(
    ctx: &mut BuilderContext,
    mut args: Vec<PointerValue>,
) -> Vec<PointerValue> {
    if args.len() < 2 {
        let mut phase_offset_constant = NumValue::new_undef(ctx.context, ctx.allocb);
        phase_offset_constant.store(ctx.b, &NumValue::get_const(ctx.context, 0., 0., 0));
        args.push(phase_offset_constant.val);
    }
    args
}

fn periodic_data_type(context: &Context) -> StructType {
    context.struct_type(&[&context.f32_type().vec_type(2)], false)
}

fn gen_periodic_call(
    func: &mut FunctionContext,
    args: &[PointerValue],
    result: PointerValue,
    next_val: &Fn(&mut FunctionContext, VectorValue) -> VectorValue,
) {
    let phase_ptr = unsafe { func.ctx.b.build_struct_gep(&func.data_ptr, 0, "phase.ptr") };

    let freq_num = NumValue::new(args[0]);
    let phase_offset_num = NumValue::new(args[1]);
    let result_num = NumValue::new(result);

    let freq_vec = freq_num.get_vec(func.ctx.b);

    // offset phase and store new value
    let phase = func
        .ctx
        .b
        .build_load(&phase_ptr, "phase")
        .into_vector_value();
    let samplerate = func
        .ctx
        .b
        .build_load(
            &globals::get_sample_rate(func.ctx.module).as_pointer_value(),
            "samplerate",
        )
        .into_vector_value();
    let phase_offset = func
        .ctx
        .b
        .build_float_div(freq_vec, samplerate, "phaseoffset");
    let new_phase = func.ctx.b.build_float_add(phase, phase_offset, "newphase");
    let mod_phase = func.ctx.b.build_float_rem(
        new_phase,
        util::get_vec_spread(func.ctx.context, 2.),
        "modphase",
    );
    func.ctx.b.build_store(&phase_ptr, &mod_phase);

    // calculate result
    let phase_offset_vec = phase_offset_num.get_vec(func.ctx.b);
    let input_phase = func
        .ctx
        .b
        .build_float_add(phase_offset_vec, phase, "inputphase");
    let result_vec = next_val(func, input_phase);

    result_num.set_vec(func.ctx.b, &result_vec);
    result_num.set_form(
        func.ctx.b,
        &func
            .ctx
            .context
            .i8_type()
            .const_int(FormType::Oscillator as u64, false),
    );
}

macro_rules! define_periodic_func (
    ($func_name:ident: $func_type:expr => $callback:expr) => (
        pub struct $func_name {}
        impl Function for $func_name {
            fn function_type() -> block::Function { $func_type }
            fn gen_real_args(ctx: &mut BuilderContext, args: Vec<PointerValue>) -> Vec<PointerValue> {
                gen_periodic_real_args(ctx, args)
            }
            fn data_type(context: &Context) -> StructType {
                periodic_data_type(context)
            }
            fn gen_call(func: &mut FunctionContext, args: &[PointerValue], _varargs: Option<VarArgs>, result: PointerValue) {
                gen_periodic_call(func, args, result, &$callback)
            }
        }
    )
);

fn sin_next_value(func: &mut FunctionContext, phase: VectorValue) -> VectorValue {
    let sin_intrinsic = intrinsics::sin_v2f32(func.ctx.module);
    let sin_phase = func.ctx.b.build_float_mul(
        phase,
        util::get_vec_spread(func.ctx.context, consts::PI * 2.),
        "sinphase",
    );
    func.ctx
        .b
        .build_call(&sin_intrinsic, &[&sin_phase], "result", false)
        .left()
        .unwrap()
        .into_vector_value()
}
define_periodic_func!(SinOscFunction: block::Function::SinOsc => sin_next_value);

fn sqr_next_value(func: &mut FunctionContext, phase: VectorValue) -> VectorValue {
    let floor_intrinsic = intrinsics::floor_v2f32(func.ctx.module);

    let normal_period = util::get_vec_spread(func.ctx.context, 2.);
    func.ctx.b.build_float_sub(
        func.ctx.b.build_float_mul(
            func.ctx
                .b
                .build_call(
                    &floor_intrinsic,
                    &[&func.ctx.b.build_float_rem(
                        func.ctx
                            .b
                            .build_float_mul(phase, normal_period, "input_val"),
                        normal_period,
                        "inputmod",
                    )],
                    "inputfloor",
                    false,
                )
                .left()
                .unwrap()
                .into_vector_value(),
            util::get_vec_spread(func.ctx.context, 2.),
            "normalized",
        ),
        util::get_vec_spread(func.ctx.context, 1.),
        "result",
    )
}
define_periodic_func!(SqrOscFunction: block::Function::SqrOsc => sqr_next_value);

fn saw_next_value(func: &mut FunctionContext, phase: VectorValue) -> VectorValue {
    let normal_period = util::get_vec_spread(func.ctx.context, 2.);
    func.ctx.b.build_float_sub(
        func.ctx.b.build_float_rem(
            func.ctx.b.build_float_mul(phase, normal_period, "inputval"),
            normal_period,
            "inputmod",
        ),
        util::get_vec_spread(func.ctx.context, 1.),
        "result",
    )
}
define_periodic_func!(SawOscFunction: block::Function::SawOsc => saw_next_value);

fn tri_next_value(func: &mut FunctionContext, phase: VectorValue) -> VectorValue {
    let abs_intrinsic = intrinsics::fabs_v2f32(func.ctx.module);

    let normal_period = util::get_vec_spread(func.ctx.context, 4.);
    func.ctx.b.build_float_sub(
        func.ctx
            .b
            .build_call(
                &abs_intrinsic,
                &[&func.ctx.b.build_float_sub(
                    func.ctx.b.build_float_rem(
                        func.ctx.b.build_float_mul(phase, normal_period, "inputval"),
                        normal_period,
                        "inputmod",
                    ),
                    util::get_vec_spread(func.ctx.context, 2.),
                    "inputsub",
                )],
                "normalized",
                false,
            )
            .left()
            .unwrap()
            .into_vector_value(),
        util::get_vec_spread(func.ctx.context, 1.),
        "result",
    )
}
define_periodic_func!(TriOscFunction: block::Function::TriOsc => tri_next_value);

fn rmp_next_value(func: &mut FunctionContext, phase: VectorValue) -> VectorValue {
    let normal_period = util::get_vec_spread(func.ctx.context, 2.);
    func.ctx.b.build_float_sub(
        util::get_vec_spread(func.ctx.context, 1.),
        func.ctx.b.build_float_rem(
            func.ctx.b.build_float_mul(phase, normal_period, "inputval"),
            normal_period,
            "inputmod",
        ),
        "result",
    )
}
define_periodic_func!(RmpOscFunction: block::Function::RmpOsc => rmp_next_value);
