use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::types::StructType;
use inkwell::values::{IntValue, PointerValue};

pub const ARRAY_CAPACITY: u8 = 32;

#[derive(Debug, Clone)]
pub struct ArrayValue {
    pub val: PointerValue,
}

impl ArrayValue {
    pub fn get_type(context: &Context, inner_type: StructType) -> StructType {
        context.struct_type(
            &[
                &context.i32_type(),
                &inner_type.array_type(u32::from(ARRAY_CAPACITY)),
            ],
            false,
        )
    }

    pub fn new(val: PointerValue) -> Self {
        ArrayValue { val }
    }

    pub fn get_bitmap_ptr(&self, builder: &mut Builder) -> PointerValue {
        unsafe { builder.build_struct_gep(&self.val, 0, "array.bitmap.ptr") }
    }

    pub fn get_bitmap(&self, builder: &mut Builder) -> IntValue {
        let ptr = self.get_bitmap_ptr(builder);
        builder.build_load(&ptr, "array.bitmap").into_int_value()
    }

    pub fn set_bitmap(&self, builder: &mut Builder, val: IntValue) {
        let ptr = self.get_bitmap_ptr(builder);
        builder.build_store(&ptr, &val);
    }

    pub fn get_items_ptr(&self, builder: &mut Builder) -> PointerValue {
        unsafe { builder.build_struct_gep(&self.val, 1, "array.items.ptr") }
    }

    pub fn get_item_ptr(&self, builder: &mut Builder, index: IntValue) -> PointerValue {
        let context = self.val.get_type().get_context();
        unsafe {
            builder.build_in_bounds_gep(
                &self.val,
                &[
                    context.i64_type().const_int(0, false),
                    context.i32_type().const_int(1, false),
                    index,
                ],
                "array.item",
            )
        }
    }
}
