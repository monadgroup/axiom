use inkwell::values::{StructValue, VectorValue, IntValue, FloatValue, PointerValue, BasicValueEnum};
use inkwell::types::{BasicTypeEnum, StructType, IntType, VectorType};
use inkwell::context::Context;
use inkwell::builder::Builder;

#[derive(Debug, Clone)]
pub struct NumValue {
    pub val: PointerValue,
}

impl NumValue {
    pub fn get_type(context: &Context) -> StructType {
        let struct_type = context.opaque_struct_type("struct.num");
        struct_type.set_body(
            &[
                &BasicTypeEnum::from(context.f32_type().vec_type(2)),
                &BasicTypeEnum::from(context.i8_type())
            ],
            false
        );
        struct_type
    }

    pub fn new(val: PointerValue) -> Self {
        NumValue { val }
    }

    pub fn new_undef(context: &Context, alloca_builder: &mut Builder) -> Self {
        let num_type = NumValue::get_type(context);
        NumValue::new(alloca_builder.build_alloca(&num_type, "num"))
    }

    pub fn clone(context: &Context, alloca_builder: &mut Builder, builder: &mut Builder, val: &StructValue) -> Self {
        let mut new_val = NumValue::new_undef(context, alloca_builder);
        new_val.store(builder, val);
        new_val
    }

    pub fn get_const(context: &Context, left: f64, right: f64, form: u8) -> StructValue {
        NumValue::get_type(context).const_named_struct(&[
            &BasicValueEnum::from(VectorType::const_vector(&[
                &context.f32_type().const_float(left),
                &context.f32_type().const_float(right)
            ])),
            &BasicValueEnum::from(context.i8_type().const_int(form as u64, false))
        ])
    }

    pub fn load(&self, builder: &mut Builder) -> StructValue {
        builder.build_load(&self.val, "num.loaded").into_struct_value()
    }

    pub fn store(&mut self, builder: &mut Builder, value: &StructValue) {
        builder.build_store(&self.val, value);
    }

    pub fn get_vec_ptr(&self, builder: &mut Builder) -> PointerValue {
        unsafe {
            builder.build_struct_gep(&self.val, 0, "num.vec.ptr")
        }
    }

    pub fn get_vec(&self, builder: &mut Builder) -> VectorValue {
        let vec = self.get_vec_ptr(builder);
        builder.build_load(&vec, "num.vec").into_vector_value()
    }

    pub fn set_vec(&mut self, builder: &mut Builder, value: &VectorValue) {
        let vec = self.get_vec_ptr(builder);
        builder.build_store(&vec, value);
    }

    pub fn get_form_ptr(&self, builder: &mut Builder) -> PointerValue {
        unsafe {
            builder.build_struct_gep(&self.val, 1, "num.form.ptr")
        }
    }

    pub fn get_form(&self, builder: &mut Builder) -> IntValue {
        let vec = self.get_form_ptr(builder);
        builder.build_load(&vec, "num.form").into_int_value()
    }

    pub fn set_form(&mut self, builder: &mut Builder, value: &IntValue) {
        let vec = self.get_form_ptr(builder);
        builder.build_store(&vec, value);
    }
}
