use super::{Function, FunctionContext, VarArgs};
use crate::ast::FormType;
use crate::codegen::values::NumValue;
use crate::codegen::{globals, math, util, BuilderContext};
use crate::mir::block;
use inkwell::context::Context;
use inkwell::types::StructType;
use inkwell::values::{PointerValue, VectorValue};
use inkwell::FloatPredicate;
use std::f64::consts;

fn gen_periodic_real_args(
    ctx: &mut BuilderContext,
    mut args: Vec<PointerValue>,
    needs_pulse_width: bool,
) -> Vec<PointerValue> {
    if args.len() < 2 {
        let mut phase_offset_constant = NumValue::new_undef(ctx.context, ctx.allocb);
        phase_offset_constant.store(ctx.b, NumValue::get_const(ctx.context, 0., 0., 0));
        args.push(phase_offset_constant.val);
    }
    if needs_pulse_width && args.len() < 3 {
        let mut pulse_width_constant = NumValue::new_undef(ctx.context, ctx.allocb);
        pulse_width_constant.store(ctx.b, NumValue::get_const(ctx.context, 0.5, 0.5, 0));
        args.push(pulse_width_constant.val);
    }
    args
}

fn periodic_data_type(context: &Context) -> StructType {
    context.struct_type(&[&context.f64_type().vec_type(2)], false)
}

fn gen_periodic_call(
    func: &mut FunctionContext,
    args: &[PointerValue],
    result: PointerValue,
    next_val: &Fn(&mut FunctionContext, VectorValue, &[PointerValue]) -> VectorValue,
) {
    let fract_intrinsic = math::fract_v2f64(func.ctx.module);

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
    let mod_phase = func
        .ctx
        .b
        .build_call(&fract_intrinsic, &[&new_phase], "modphase", true)
        .left()
        .unwrap()
        .into_vector_value();
    func.ctx.b.build_store(&phase_ptr, &mod_phase);

    // calculate result
    let phase_offset_vec = phase_offset_num.get_vec(func.ctx.b);
    let input_phase = func
        .ctx
        .b
        .build_call(
            &fract_intrinsic,
            &[&func
                .ctx
                .b
                .build_float_add(phase_offset_vec, phase, "inputphase")],
            "inputphasemod",
            true,
        )
        .left()
        .unwrap()
        .into_vector_value();

    let result_vec = next_val(func, input_phase, &args[2..]);

    result_num.set_vec(func.ctx.b, result_vec);
    result_num.set_form(
        func.ctx.b,
        func.ctx
            .context
            .i8_type()
            .const_int(FormType::Oscillator as u64, false),
    );
}

macro_rules! define_periodic_func (
    ($func_name:ident: $func_type:expr, $needs_pulse_width:expr => $callback:expr) => (
        pub struct $func_name {}
        impl Function for $func_name {
            fn function_type() -> block::Function { $func_type }
            fn gen_real_args(ctx: &mut BuilderContext, args: Vec<PointerValue>) -> Vec<PointerValue> {
                gen_periodic_real_args(ctx, args, $needs_pulse_width)
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

fn sin_next_value(
    func: &mut FunctionContext,
    phase: VectorValue,
    _extra_args: &[PointerValue],
) -> VectorValue {
    let sin_intrinsic = math::sin_v2f64(func.ctx.module);
    let sin_phase = func.ctx.b.build_float_mul(
        phase,
        util::get_vec_spread(func.ctx.context, consts::PI * 2.),
        "sinphase",
    );
    func.ctx
        .b
        .build_call(&sin_intrinsic, &[&sin_phase], "result", true)
        .left()
        .unwrap()
        .into_vector_value()
}
define_periodic_func!(SinOscFunction: block::Function::SinOsc, false => sin_next_value);

fn sqr_next_value(
    func: &mut FunctionContext,
    phase: VectorValue,
    extra_args: &[PointerValue],
) -> VectorValue {
    let pulse_width = NumValue::new(extra_args[0]);
    let pulse_width_vec = pulse_width.get_vec(func.ctx.b);

    let is_positive =
        func.ctx
            .b
            .build_float_compare(FloatPredicate::OLT, phase, pulse_width_vec, "isneg");

    func.ctx
        .b
        .build_select(
            is_positive,
            util::get_vec_spread(func.ctx.context, 1.),
            util::get_vec_spread(func.ctx.context, -1.),
            "result",
        )
        .into_vector_value()
}
define_periodic_func!(SqrOscFunction: block::Function::SqrOsc, true => sqr_next_value);

fn saw_next_value(
    func: &mut FunctionContext,
    phase: VectorValue,
    _extra_args: &[PointerValue],
) -> VectorValue {
    func.ctx.b.build_float_sub(
        func.ctx.b.build_float_mul(
            phase,
            util::get_vec_spread(func.ctx.context, 2.),
            "inputval",
        ),
        util::get_vec_spread(func.ctx.context, 1.),
        "result",
    )
}
define_periodic_func!(SawOscFunction: block::Function::SawOsc, false => saw_next_value);

fn tri_next_value(
    func: &mut FunctionContext,
    phase: VectorValue,
    _extra_args: &[PointerValue],
) -> VectorValue {
    let abs_intrinsic = math::abs_v2f64(func.ctx.module);

    func.ctx.b.build_float_sub(
        util::get_vec_spread(func.ctx.context, 1.),
        func.ctx
            .b
            .build_call(
                &abs_intrinsic,
                &[&func.ctx.b.build_float_sub(
                    func.ctx.b.build_float_mul(
                        phase,
                        util::get_vec_spread(func.ctx.context, 4.),
                        "",
                    ),
                    util::get_vec_spread(func.ctx.context, 2.),
                    "",
                )],
                "normalized",
                true,
            )
            .left()
            .unwrap()
            .into_vector_value(),
        "result",
    )
}
define_periodic_func!(TriOscFunction: block::Function::TriOsc, false => tri_next_value);

fn rmp_next_value(
    func: &mut FunctionContext,
    phase: VectorValue,
    _extra_args: &[PointerValue],
) -> VectorValue {
    func.ctx.b.build_float_sub(
        util::get_vec_spread(func.ctx.context, 1.),
        func.ctx.b.build_float_mul(
            phase,
            util::get_vec_spread(func.ctx.context, 2.),
            "inputval",
        ),
        "result",
    )
}
define_periodic_func!(RmpOscFunction: block::Function::RmpOsc, false => rmp_next_value);
