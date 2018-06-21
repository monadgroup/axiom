use inkwell::values::{PointerValue, IntValue, StructValue, BasicValueEnum};
use inkwell::types::{StructType, BasicTypeEnum};
use inkwell::context::Context;
use inkwell::builder::Builder;

#[derive(Debug, Clone)]
pub struct MidiEventValue {
    pub val: PointerValue,
}

impl MidiEventValue {
    pub fn get_type(context: &Context) -> StructType {
        let struct_type = context.opaque_struct_type("struct.midievent");
        struct_type.set_body(
            &[
                &BasicTypeEnum::from(context.i8_type()), // name
                &BasicTypeEnum::from(context.i8_type()), // channel
                &BasicTypeEnum::from(context.i8_type()), // note
                &BasicTypeEnum::from(context.i8_type()), // param
            ],
            false
        );
        struct_type
    }

    pub fn new(val: PointerValue) -> Self {
        MidiEventValue { val }
    }

    pub fn get_const(context: &Context, name: u8, channel: u8, note: u8, param: u8) -> StructValue {
        MidiEventValue::get_type(context).const_named_struct(&[
            &BasicValueEnum::from(context.i8_type().const_int(name as u64, false)),
            &BasicValueEnum::from(context.i8_type().const_int(channel as u64, false)),
            &BasicValueEnum::from(context.i8_type().const_int(note as u64, false)),
            &BasicValueEnum::from(context.i8_type().const_int(param as u64, false))
        ])
    }

    pub fn load(&self, builder: &mut Builder) -> StructValue {
        builder.build_load(&self.val, "midi.event").into_struct_value()
    }

    pub fn store(&mut self, builder: &mut Builder, value: &StructValue) {
        builder.build_store(&self.val, value);
    }

    pub fn get_name_ptr(&self, builder: &mut Builder) -> PointerValue {
        unsafe {
            builder.build_struct_gep(&self.val, 0, "event.name.ptr")
        }
    }

    pub fn get_name(&self, builder: &mut Builder) -> IntValue {
        let ptr = self.get_name_ptr(builder);
        builder.build_load(&ptr, "event.name").into_int_value()
    }

    pub fn get_channel_ptr(&self, builder: &mut Builder) -> PointerValue {
        unsafe {
            builder.build_struct_gep(&self.val, 1, "event.channel.ptr")
        }
    }

    pub fn get_channel(&self, builder: &mut Builder) -> IntValue {
        let ptr = self.get_channel_ptr(builder);
        builder.build_load(&ptr, "event.channel").into_int_value()
    }

    pub fn get_note_ptr(&self, builder: &mut Builder) -> PointerValue {
        unsafe {
            builder.build_struct_gep(&self.val, 2, "event.note.ptr")
        }
    }

    pub fn get_note(&self, builder: &mut Builder) -> IntValue {
        let ptr = self.get_note_ptr(builder);
        builder.build_load(&ptr, "event.note").into_int_value()
    }

    pub fn get_param_ptr(&self, builder: &mut Builder) -> PointerValue {
        unsafe {
            builder.build_struct_gep(&self.val, 3, "event.param.ptr")
        }
    }

    pub fn get_param(&self, builder: &mut Builder) -> IntValue {
        let ptr = self.get_param_ptr(builder);
        builder.build_load(&ptr, "event.param").into_int_value()
    }
}
