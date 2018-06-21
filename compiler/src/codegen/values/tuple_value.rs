use inkwell::values::{PointerValue, StructValue, BasicValue, IntValue, BasicValueEnum};
use inkwell::types::{BasicTypeEnum, BasicType};
use inkwell::context::Context;
use inkwell::builder::Builder;
use inkwell::module::Module;
use codegen::util;

#[derive(Debug, Clone)]
pub struct TupleValue {
    pub val: PointerValue,
}

impl TupleValue {
    pub fn new(val: PointerValue) -> Self {
        TupleValue { val }
    }

    pub fn new_from(module: &Module, alloca_builder: &mut Builder, builder: &mut Builder, values: &[PointerValue]) -> TupleValue {
        let enum_types: Vec<_> = values.iter().map(|val| { val.get_type().element_type() }).collect();
        let inner_types: Vec<_> = enum_types.iter().map(|val| { val as &BasicType }).collect();
        let new_type = module.get_context().struct_type(&inner_types, false);
        let new_tuple = TupleValue::new(alloca_builder.build_alloca(&new_type, "tuple"));

        for (index, val) in values.iter().enumerate() {
            let target_ptr = new_tuple.get_item_ptr(builder, index as u32);
            util::copy_ptr(builder, module, val, &target_ptr);
        }

        new_tuple
    }

    pub fn get_const(context: &Context, values: &[&BasicValue]) -> StructValue {
        context.const_struct(values, false)
    }

    pub fn get_item_ptr(&self, builder: &mut Builder, index: u32) -> PointerValue {
        let context = self.val.get_type().get_context();
        unsafe {
            builder.build_struct_gep(&self.val, index, "tuple.item.ptr")
        }
    }
}
