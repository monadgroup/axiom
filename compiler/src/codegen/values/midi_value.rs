use super::MidiEventValue;
use codegen::util;
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicTypeEnum, StructType};
use inkwell::values::{FunctionValue, IntValue, PointerValue};
use inkwell::AddressSpace;
use inkwell::IntPredicate;
use std::borrow::Borrow;

pub const MIDI_EVENT_COUNT: u8 = 16;

#[derive(Debug, Clone)]
pub struct MidiValue {
    pub val: PointerValue,
}

impl MidiValue {
    pub fn get_type(context: &Context) -> StructType {
        let event_type = MidiEventValue::get_type(context);
        let struct_type = context.opaque_struct_type("struct.midi");
        struct_type.set_body(
            &[
                &BasicTypeEnum::from(context.i8_type()),
                &BasicTypeEnum::from(event_type.array_type(MIDI_EVENT_COUNT as u32)),
            ],
            false,
        );
        struct_type
    }

    pub fn new(val: PointerValue) -> Self {
        MidiValue { val }
    }

    pub fn new_undef(context: &Context, alloca_builder: &mut Builder) -> Self {
        let midi_type = MidiValue::get_type(context);
        MidiValue::new(alloca_builder.build_alloca(&midi_type, "midi"))
    }

    pub fn copy_to(&self, builder: &mut Builder, module: &Module, other: &MidiValue) {
        util::copy_ptr(builder, module, &self.val, &other.val)
    }

    pub fn get_count_ptr(&self, builder: &mut Builder) -> PointerValue {
        unsafe { builder.build_struct_gep(&self.val, 0, "midi.count.ptr") }
    }

    pub fn get_count(&self, builder: &mut Builder) -> IntValue {
        let ptr = self.get_count_ptr(builder);
        builder.build_load(&ptr, "midi.count").into_int_value()
    }

    pub fn set_count(&self, builder: &mut Builder, val: &IntValue) {
        let ptr = self.get_count_ptr(builder);
        builder.build_store(&ptr, val);
    }

    pub fn get_events_ptr(&self, builder: &mut Builder) -> PointerValue {
        unsafe { builder.build_struct_gep(&self.val, 1, "midi.events.ptr") }
    }

    pub fn get_event(&self, builder: &mut Builder, index: IntValue) -> MidiEventValue {
        let context = self.val.get_type().get_context();
        let ptr = unsafe {
            builder.build_in_bounds_gep(
                &self.val,
                &[
                    context.i64_type().const_int(0, false),
                    context.i32_type().const_int(1, false),
                    index,
                ],
                "midi.event",
            )
        };
        MidiEventValue::new(ptr)
    }

    pub fn push_event(&self, builder: &mut Builder, module: &Module, event: &MidiEventValue) {
        let push_func = MidiValue::get_push_event_func(module, module.get_context().borrow());
        builder.build_call(&push_func, &[&self.val, &event.val], "", false);
    }

    fn get_push_event_func(module: &Module, context: &Context) -> FunctionValue {
        util::get_or_create_func(
            module,
            "maxim.midi.pushEvent",
            &context.void_type().fn_type(
                &[
                    &BasicTypeEnum::from(
                        MidiValue::get_type(context).ptr_type(AddressSpace::Local),
                    ),
                    &BasicTypeEnum::from(
                        MidiEventValue::get_type(context).ptr_type(AddressSpace::Local),
                    ),
                ],
                false,
            ),
            Some(&Linkage::ExternalLinkage),
        )
    }

    pub fn initialize(module: &Module, context: &Context) {
        let func = MidiValue::get_push_event_func(module, context);
        let entry_block = func.append_basic_block("entry");
        let can_push_block = func.append_basic_block("canpush");
        let end_block = func.append_basic_block("end");

        let mut builder = context.create_builder();
        builder.position_at_end(&entry_block);

        let current_midi = MidiValue::new(func.get_nth_param(0).unwrap().into_pointer_value());
        let push_evt = MidiEventValue::new(func.get_nth_param(1).unwrap().into_pointer_value());

        let current_count = current_midi.get_count(&mut builder);
        let can_push_cond = builder.build_int_compare(
            IntPredicate::ULT,
            current_count,
            context.i8_type().const_int(MIDI_EVENT_COUNT as u64, false),
            "canpushcond",
        );
        builder.build_conditional_branch(&can_push_cond, &can_push_block, &end_block);
        builder.position_at_end(&can_push_block);

        let current_event = current_midi.get_event(&mut builder, current_count);
        push_evt.copy_to(&mut builder, module, &current_event);
        let new_count = builder.build_int_add(
            current_count,
            context.i8_type().const_int(1, false),
            "newcount",
        );
        current_midi.set_count(&mut builder, &new_count);
        builder.build_unconditional_branch(&end_block);
        builder.position_at_end(&end_block);
        builder.build_return(None);
    }
}
