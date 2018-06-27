mod function_context;
mod vector_intrinsic_function;

use codegen::LifecycleFunc;
use codegen::{util, values, BuilderContext};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicType, BasicTypeEnum, StructType};
use inkwell::values::{BasicValue, BasicValueEnum, FunctionValue, IntValue, PointerValue};
use inkwell::AddressSpace;
use mir::{block, VarType};
use std::{fmt, ops};

use self::function_context::FunctionContext;

pub use self::vector_intrinsic_function::*;

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

pub trait VarArgs {
    fn len(&self, builder: &mut Builder) -> IntValue;
    fn index(&self, index: IntValue) -> &PointerValue;
}

impl ops::Index<IntValue> for VarArgs {
    type Output = PointerValue;

    fn index(&self, index: IntValue) -> &PointerValue {
        self.index(index)
    }
}

struct InternalVarArgs {}

impl VarArgs for InternalVarArgs {
    fn len(&self, builder: &mut Builder) -> IntValue {
        unimplemented!()
    }

    fn index(&self, index: IntValue) -> &PointerValue {
        unimplemented!()
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
                _ => unimplemented!()
            }
        }

        fn map_real_args(function_type: block::Function, ctx: &mut BuilderContext, args: Vec<PointerValue>) -> Vec<PointerValue> {
            match function_type {
                $( block::Function::$enum_name => $class_name::gen_real_args(ctx, args), )*
                _ => unimplemented!()
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
    Abs => AbsFunction
}

fn get_lifecycle_func(
    module: &Module,
    function: block::Function,
    lifecycle: FunctionLifecycleFunc,
) -> FunctionValue {
    let func_name = format!("maxim.function.{}.{}", function, lifecycle);
    util::get_or_create_func(module, &func_name, &|| {
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
    util::get_or_create_func(module, &func_name, &|| {
        let context = module.get_context();
        let mut arg_types: Vec<BasicTypeEnum> = vec![
            get_data_type(&context, function)
                .ptr_type(AddressSpace::Generic)
                .into(),
            get_return_type(&context, function)
                .ptr_type(AddressSpace::Generic)
                .into(),
        ];

        // add all argument types
        arg_types.extend(function.arg_types().iter().map(|param| -> BasicTypeEnum {
            values::remap_type(&context, &param.value_type)
                .ptr_type(AddressSpace::Generic)
                .into()
        }));

        // add the vararg if it exists
        if let Some(vararg_type) = function.var_arg() {
            arg_types.push(
                context
                    .struct_type(
                        &[
                            &context.i8_type(),
                            &values::remap_type(&context, &vararg_type)
                                .ptr_type(AddressSpace::Generic)
                                .ptr_type(AddressSpace::Generic),
                        ],
                        false,
                    )
                    .into(),
            )
        }

        let arg_refs: Vec<_> = arg_types
            .iter()
            .map(|arg_type| arg_type as &BasicType)
            .collect();
        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(&arg_refs, false),
        )
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
    builder.build_call(&func, &[&data_ptr], "", false);
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
    let mut pass_args: Vec<BasicValueEnum> = vec![data_ptr.into(), out_val.into()];
    pass_args.extend(
        map_real_args(function, ctx, args)
            .into_iter()
            .map(|ptr| BasicValueEnum::from(ptr)),
    );

    // build the vararg struct
    if let Some(vararg_type) = function.var_arg() {
        let ll_vararg_type =
            values::remap_type(&ctx.context, &vararg_type).ptr_type(AddressSpace::Generic);
        let va_struct_type = ctx.context.struct_type(
            &[
                &ctx.context.i8_type(),
                &ll_vararg_type.ptr_type(AddressSpace::Generic),
            ],
            false,
        );
        let va_array_type = ll_vararg_type.array_type(varargs.len() as u32);
        let va_array = ctx.allocb.build_alloca(&va_array_type, "va.arr");

        // build the vararg struct with size and pointing to the array we allocated
        let va_struct = ctx.b.build_insert_value(
            va_struct_type.get_undef(),
            &ctx.context.i8_type().const_int(varargs.len() as u64, false),
            0,
            "va.withsize",
        );
        let va_struct = ctx.b.build_insert_value(va_struct, &va_array, 1, "va");

        // fill the array with the provided pointers
        for (vararg_index, vararg_ptr) in varargs.iter().enumerate() {
            let store_pos = unsafe {
                ctx.b
                    .build_struct_gep(&va_array, vararg_index as u32, "va.arr.itemptr")
            };
            ctx.b.build_store(&store_pos, vararg_ptr);
        }

        pass_args.push(va_struct.into());
    }

    let pass_arg_refs: Vec<_> = pass_args.iter().map(|arg| arg as &BasicValue).collect();
    ctx.b.build_call(&func, &pass_arg_refs, "", false);
}

pub trait Function {
    fn function_type() -> block::Function;

    fn data_type(context: &Context) -> StructType {
        context.struct_type(&[], false)
    }

    fn gen_construct(func: &mut FunctionContext) {}

    fn gen_real_args(ctx: &mut BuilderContext, args: Vec<PointerValue>) -> Vec<PointerValue> {
        args
    }

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        varargs: &VarArgs,
        result: PointerValue,
    );

    fn gen_destruct(func: &mut FunctionContext) {}
}
