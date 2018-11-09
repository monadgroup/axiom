mod biquad_filter_function;
mod channel_function;
mod defer_function;
mod delay_function;
mod function_context;
mod indexed_function;
mod note_function;
mod num_function;
mod oscillator_function;
mod scalar_intrinsic_function;
mod sv_filter_function;
mod vector_intrinsic_function;
mod vector_shuffle_function;
mod voices_function;

use codegen::{build_context_function, util, values, BuilderContext, TargetProperties};
use inkwell::attribute::AttrKind;
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicType, BasicTypeEnum, StructType};
use inkwell::values::{
    BasicValue, BasicValueEnum, FunctionValue, InstructionOpcode, IntValue, PointerValue,
    StructValue,
};
use inkwell::AddressSpace;
use mir::{block, VarType};
use std::fmt;

use self::function_context::FunctionContext;

pub use self::biquad_filter_function::*;
pub use self::channel_function::*;
pub use self::defer_function::*;
pub use self::delay_function::*;
pub use self::indexed_function::*;
pub use self::note_function::*;
pub use self::num_function::*;
pub use self::oscillator_function::*;
pub use self::scalar_intrinsic_function::*;
pub use self::sv_filter_function::*;
pub use self::vector_intrinsic_function::*;
pub use self::vector_shuffle_function::*;
pub use self::voices_function::*;
pub use self::voices_function::*;

pub enum FunctionLifecycleFunc {
    Construct,
    Destruct,
}

impl fmt::Display for FunctionLifecycleFunc {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            FunctionLifecycleFunc::Construct => write!(f, "construct"),
            FunctionLifecycleFunc::Destruct => write!(f, "destruct"),
        }
    }
}

pub struct VarArgs {
    param: StructValue,
    is_by_value: bool,
}

impl VarArgs {
    pub fn new(param: StructValue, is_by_value: bool) -> Self {
        VarArgs { param, is_by_value }
    }

    fn len(&self, builder: &Builder) -> IntValue {
        builder
            .build_extract_value(self.param, 0, "vararg.count")
            .into_int_value()
    }

    fn at(&self, index: IntValue, builder: &Builder) -> PointerValue {
        let arr_ptr = builder
            .build_extract_value(self.param, 1, "vararg.arr")
            .into_pointer_value();
        let item_ptr_ptr = unsafe { builder.build_gep(&arr_ptr, &[index], "vararg.item.ptr") };
        if self.is_by_value {
            item_ptr_ptr
        } else {
            builder
                .build_load(&item_ptr_ptr, "vararg.item")
                .into_pointer_value()
        }
    }
}

pub fn get_return_type(context: &Context, function: block::Function) -> StructType {
    values::remap_type(context, &VarType::of_function(&function))
}

macro_rules! map_functions {
    ($($enum_name:ident => $class_name:ident),*) => (
        pub fn get_data_type(context: &Context, function_type: block::Function) -> StructType {
            match function_type {
                $( block::Function::$enum_name => $class_name::data_type(context), )*
            }
        }

        pub fn build_funcs(module: &Module, target: &TargetProperties) {
            build_internal_biquad_func(module, target);

            $( $class_name::build_lifecycle_funcs(module, target); )*
        }

        fn map_real_args(function_type: block::Function, ctx: &mut BuilderContext, args: Vec<PointerValue>) -> Vec<PointerValue> {
            match function_type {
                $( block::Function::$enum_name => $class_name::gen_real_args(ctx, args), )*
            }
        }
    )
}

map_functions! {
    Cos => CosFunction,
    Sin => SinFunction,
    Log => LogFunction,
    Log2 => Log2Function,
    Log10 => Log10Function,
    Sqrt => SqrtFunction,
    Ceil => CeilFunction,
    Floor => FloorFunction,
    Abs => AbsFunction,
    Tan => TanFunction,
    Acos => AcosFunction,
    Asin => AsinFunction,
    Atan => AtanFunction,
    Atan2 => Atan2Function,
    Hypot => HypotFunction,
    ToRad => ToRadFunction,
    ToDeg => ToDegFunction,
    Clamp => ClampFunction,
    CopySign => CopySignFunction,
    Pan => PanFunction,
    Left => LeftFunction,
    Right => RightFunction,
    Swap => SwapFunction,
    Combine => CombineFunction,
    Mix => MixFunction,
    Sequence => SequenceFunction,
    Min => MinFunction,
    Max => MaxFunction,
    Next => NextFunction,
    Delay => DelayFunction,
    Amplitude => AmplitudeFunction,
    Hold => HoldFunction,
    Accum => AccumFunction,
    Mixdown => MixdownFunction,
    SvFilter => SvFilterFunction,
    Noise => NoiseFunction,
    SinOsc => SinOscFunction,
    SqrOsc => SqrOscFunction,
    SawOsc => SawOscFunction,
    TriOsc => TriOscFunction,
    RmpOsc => RmpOscFunction,
    LowBqFilter => LowBqFilterFunction,
    HighBqFilter => HighBqFilterFunction,
    BandBqFilter => BandBqFilterFunction,
    NotchBqFilter => NotchBqFilterFunction,
    AllBqFilter => AllBqFilterFunction,
    PeakBqFilter => PeakBqFilterFunction,
    Note => NoteFunction,
    Voices => VoicesFunction,
    Channel => ChannelFunction,
    Indexed => IndexedFunction
}

fn get_lifecycle_func(
    module: &Module,
    function: block::Function,
    lifecycle: FunctionLifecycleFunc,
) -> FunctionValue {
    let func_name = format!("maxim.function.{}.{}", function, lifecycle);
    util::get_or_create_func(module, &func_name, true, &|| {
        let context = module.get_context();
        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(
                &[&get_data_type(&context, function).ptr_type(AddressSpace::Generic)],
                false,
            ),
        )
    })
}

fn get_update_func(module: &Module, function: block::Function) -> FunctionValue {
    let func_name = format!("maxim.function.{}.update", function);
    let func = util::get_or_create_func(module, &func_name, true, &|| {
        let context = module.get_context();
        let mut arg_types: Vec<BasicTypeEnum> = vec![
            get_data_type(&context, function)
                .ptr_type(AddressSpace::Generic)
                .into(),
        ];

        let mir_return_type = VarType::of_function(&function);
        let pass_return_by_val = values::pass_type_by_val(&mir_return_type);
        if !pass_return_by_val {
            arg_types.push(
                values::remap_type(&context, &mir_return_type)
                    .ptr_type(AddressSpace::Generic)
                    .into(),
            );
        }

        // add all argument types
        arg_types.extend(function.arg_types().iter().map(|param| -> BasicTypeEnum {
            let base_type = values::remap_type(&context, &param.value_type);
            if values::pass_type_by_val(&param.value_type) {
                base_type.into()
            } else {
                base_type.ptr_type(AddressSpace::Generic).into()
            }
        }));

        // add the vararg if it exists
        if let Some(vararg_type) = function.var_arg() {
            let base_type = values::remap_type(&context, &vararg_type);
            let array_type_ptr = if values::pass_type_by_val(&vararg_type) {
                base_type.ptr_type(AddressSpace::Generic)
            } else {
                base_type
                    .ptr_type(AddressSpace::Generic)
                    .ptr_type(AddressSpace::Generic)
            };

            arg_types.push(
                context
                    .struct_type(&[&context.i8_type(), &array_type_ptr], false)
                    .into(),
            )
        }

        let arg_refs: Vec<_> = arg_types
            .iter()
            .map(|arg_type| arg_type as &BasicType)
            .collect();
        (
            Linkage::ExternalLinkage,
            if pass_return_by_val {
                values::remap_type(&context, &mir_return_type).fn_type(&arg_refs, false)
            } else {
                context.void_type().fn_type(&arg_refs, false)
            },
        )
    });
    if !values::pass_type_by_val(&VarType::of_function(&function)) {
        func.add_param_attribute(
            1,
            module.get_context().get_enum_attr(AttrKind::StructRet, 1),
        );
    }
    func
}

fn build_lifecycle_func(
    module: &Module,
    function: block::Function,
    target: &TargetProperties,
    lifecycle: FunctionLifecycleFunc,
    builder: &Fn(&mut FunctionContext),
) {
    let func = get_lifecycle_func(module, function, lifecycle);
    build_context_function(module, func, target, &|ctx: BuilderContext| {
        let data_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();
        let mut function_context = FunctionContext { ctx, data_ptr };
        builder(&mut function_context);

        function_context.ctx.b.build_return(None);
    })
}

fn build_update_func(
    module: &Module,
    function: block::Function,
    target: &TargetProperties,
    builder: &Fn(&mut FunctionContext, &[PointerValue], Option<VarArgs>, PointerValue),
) {
    let func = get_update_func(module, function);
    build_context_function(module, func, target, &|ctx: BuilderContext| {
        let mut params_iter = ctx.func.params();
        let data_ptr = params_iter.next().unwrap().into_pointer_value();
        let return_type = VarType::of_function(&function);
        let pass_return_by_val = values::pass_type_by_val(&return_type);

        let (param_offset_count, return_ptr) = if pass_return_by_val {
            (
                1,
                ctx.allocb
                    .build_alloca(&values::remap_type(ctx.context, &return_type), "ret.ptr"),
            )
        } else {
            (2, params_iter.next().unwrap().into_pointer_value())
        };

        let func_vararg = function.var_arg();
        let func_arg_types = function.arg_types();
        let arg_pointers: Vec<_> = params_iter
            .take(
                ctx.func.count_params() as usize
                    - param_offset_count
                    - if func_vararg.is_some() { 1 } else { 0 },
            ).zip(func_arg_types)
            .enumerate()
            .map(|(index, (val, param_type))| {
                if values::pass_type_by_val(&param_type.value_type) {
                    // build an alloca and store the passed value in it
                    let param_alloca = ctx.allocb.build_alloca(
                        &values::remap_type(ctx.context, &param_type.value_type),
                        &format!("param.{}.ptr", index),
                    );
                    ctx.b.build_store(&param_alloca, &val);
                    param_alloca
                } else {
                    val.into_pointer_value()
                }
            }).collect();

        let vararg = if let Some(vararg_type) = func_vararg {
            Some(VarArgs::new(
                ctx.func.get_last_param().unwrap().into_struct_value(),
                values::pass_type_by_val(&vararg_type),
            ))
        } else {
            None
        };

        let mut function_context = FunctionContext { ctx, data_ptr };
        builder(&mut function_context, &arg_pointers, vararg, return_ptr);

        if pass_return_by_val {
            let return_val = function_context.ctx.b.build_load(&return_ptr, "ret");
            function_context.ctx.b.build_return(Some(&return_val));
        } else {
            function_context.ctx.b.build_return(None);
        }
    })
}

pub fn build_lifecycle_call(
    module: &Module,
    builder: &mut Builder,
    function: block::Function,
    lifecycle: FunctionLifecycleFunc,
    data_ptr: PointerValue,
) {
    let func = get_lifecycle_func(module, function, lifecycle);
    builder.build_call(&func, &[&data_ptr], "", true);
}

pub fn build_call(
    ctx: &mut BuilderContext,
    function: block::Function,
    data_ptr: PointerValue,
    args: Vec<PointerValue>,
    varargs: Vec<PointerValue>,
    out_val: PointerValue,
) {
    let func = get_update_func(ctx.module, function);
    let mut pass_args: Vec<BasicValueEnum> = vec![data_ptr.into()];
    let pass_return_by_val = values::pass_type_by_val(&function.return_type());
    if !pass_return_by_val {
        pass_args.push(out_val.into());
    }

    let func_arg_types = function.arg_types();
    pass_args.extend(
        map_real_args(function, ctx, args)
            .into_iter()
            .zip(func_arg_types.into_iter())
            .enumerate()
            .map(|(index, (ptr, param_type))| -> BasicValueEnum {
                if values::pass_type_by_val(&param_type.value_type) {
                    ctx.b.build_load(&ptr, &format!("param.{}", index)).into()
                } else {
                    ptr.into()
                }
            }),
    );

    // build the vararg struct
    if let Some(vararg_type) = function.var_arg() {
        let base_ll_vararg_type = values::remap_type(&ctx.context, &vararg_type);
        let is_by_value = values::pass_type_by_val(&vararg_type);
        let (ll_vararg_ptr_type, ll_vararg_arr_type) = if is_by_value {
            (
                base_ll_vararg_type.ptr_type(AddressSpace::Generic),
                base_ll_vararg_type.array_type(varargs.len() as u32),
            )
        } else {
            let ptr_type = base_ll_vararg_type.ptr_type(AddressSpace::Generic);
            (
                ptr_type.ptr_type(AddressSpace::Generic),
                ptr_type.array_type(varargs.len() as u32),
            )
        };
        let va_struct_type = ctx
            .context
            .struct_type(&[&ctx.context.i8_type(), &ll_vararg_ptr_type], false);
        let va_array = ctx.allocb.build_alloca(&ll_vararg_arr_type, "va.arr");

        // the struct accepts a pointer to a pointer, which we can get by bitcasting
        let va_array_ptr = ctx.b.build_cast(
            InstructionOpcode::BitCast,
            &va_array,
            &ll_vararg_ptr_type,
            "",
        );

        // build the vararg struct with size and pointing to the array we allocated
        let va_struct = ctx.b.build_insert_value(
            va_struct_type.get_undef(),
            &ctx.context.i8_type().const_int(varargs.len() as u64, false),
            0,
            "va.withsize",
        );
        let va_struct = ctx.b.build_insert_value(va_struct, &va_array_ptr, 1, "va");

        // fill the array with the provided pointers
        for (vararg_index, vararg_ptr) in varargs.iter().enumerate() {
            let store_pos = unsafe {
                ctx.b
                    .build_struct_gep(&va_array, vararg_index as u32, "va.arr.itemptr")
            };
            if is_by_value {
                util::copy_ptr(ctx.b, ctx.module, *vararg_ptr, store_pos);
            } else {
                ctx.b.build_store(&store_pos, vararg_ptr);
            }
        }

        pass_args.push(va_struct.into());
    }

    let pass_arg_refs: Vec<_> = pass_args.iter().map(|arg| arg as &BasicValue).collect();
    let ret_val = ctx.b.build_call(&func, &pass_arg_refs, "", true);
    if pass_return_by_val {
        ctx.b.build_store(&out_val, &ret_val.left().unwrap());
    }
}

pub trait Function {
    fn function_type() -> block::Function;

    fn data_type(context: &Context) -> StructType {
        context.struct_type(&[], false)
    }

    fn gen_construct(_func: &mut FunctionContext) {}

    fn gen_real_args(_ctx: &mut BuilderContext, args: Vec<PointerValue>) -> Vec<PointerValue> {
        args
    }

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        varargs: Option<VarArgs>,
        result: PointerValue,
    );

    fn gen_destruct(_func: &mut FunctionContext) {}

    fn build_lifecycle_funcs(module: &Module, target: &TargetProperties) {
        build_lifecycle_func(
            module,
            Self::function_type(),
            target,
            FunctionLifecycleFunc::Construct,
            &Self::gen_construct,
        );
        build_update_func(module, Self::function_type(), target, &Self::gen_call);
        build_lifecycle_func(
            module,
            Self::function_type(),
            target,
            FunctionLifecycleFunc::Destruct,
            &Self::gen_destruct,
        );
    }
}
