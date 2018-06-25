mod function_context;
mod vector_intrinsic_function;

use codegen::LifecycleFunc;
use codegen::{util, values};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::types::StructType;
use inkwell::values::{FunctionValue, IntValue, PointerValue};
use mir::{block, VarType};
use std::{fmt, ops};

use self::function_context::FunctionContext;

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

pub fn get_return_type(context: &Context, function: &block::Function) -> StructType {
    values::remap_type(context, &VarType::of_function(function))
}

fn get_lifecycle_func(
    module: &Module,
    builder: &mut Builder,
    function: &block::Function,
    lifecycle: FunctionLifecycleFunc,
) -> FunctionValue {
    //let func_name = format!("maxim.function.{}.{}", function, lifecycle);
    unimplemented!()
}

pub fn build_lifecycle_call(
    module: &Module,
    builder: &mut Builder,
    function: &block::Function,
    lifecycle: FunctionLifecycleFunc,
    data_ptr: PointerValue,
) {
    unimplemented!()
}

pub fn build_call(
    module: &Module,
    builder: &mut Builder,
    function: &block::Function,
    data_ptr: PointerValue,
    args: &[PointerValue],
    varargs: &[PointerValue],
    out_val: PointerValue,
) {
    unimplemented!()
}

pub trait Function {
    fn function_type() -> block::Function;

    fn data_type(context: &Context) -> StructType {
        context.struct_type(&[], false)
    }

    fn gen_construct(func: &mut FunctionContext) {}

    fn gen_call(
        func: &mut FunctionContext,
        args: &[PointerValue],
        varargs: &VarArgs,
        result: PointerValue,
    );

    fn gen_destruct(func: &mut FunctionContext) {}
}
