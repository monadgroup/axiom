use super::MidiEventValue;
use codegen::{build_context_function, util, BuilderContext, TargetProperties};
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::StructType;
use inkwell::values::{FunctionValue, IntValue, PointerValue};
use inkwell::AddressSpace;
use inkwell::IntPredicate;

pub const MIDI_EVENT_COUNT: u8 = 16;

#[derive(Debug, Clone)]
pub struct MidiValue {
    pub val: PointerValue,
}

impl MidiValue {
    pub fn get_type(context: &Context) -> StructType {
        let event_type = MidiEventValue::get_type(context);
        context.struct_type(
            &[
                &context.i8_type(),
                &event_type.array_type(MIDI_EVENT_COUNT as u32),
            ],
            false,
        )
    }

    pub fn new(val: PointerValue) -> Self {
        MidiValue { val }
    }

    pub fn new_undef(context: &Context, alloca_builder: &mut Builder) -> Self {
        let midi_type = MidiValue::get_type(context);
        MidiValue::new(alloca_builder.build_alloca(&midi_type, "midi"))
    }

    pub fn copy_to(&self, builder: &mut Builder, module: &Module, other: &MidiValue) {
        util::copy_ptr(builder, module, self.val, other.val)
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
        let push_func = MidiValue::get_push_event_func(module);
        builder.build_call(&push_func, &[&self.val, &event.val], "", true);
    }

    fn get_push_event_func(module: &Module) -> FunctionValue {
        util::get_or_create_func(module, "maxim.midi.pushEvent", true, &|| {
            let context = module.get_context();
            (
                Linkage::ExternalLinkage,
                context.void_type().fn_type(
                    &[
                        &MidiValue::get_type(&context).ptr_type(AddressSpace::Generic),
                        &MidiEventValue::get_type(&context).ptr_type(AddressSpace::Generic),
                    ],
                    false,
                ),
            )
        })
    }

    pub fn initialize(module: &Module, target: &TargetProperties) {
        build_context_function(
            module,
            MidiValue::get_push_event_func(module),
            target,
            &|ctx: BuilderContext| {
                let can_push_block = ctx.func.append_basic_block("canpush");
                let end_block = ctx.func.append_basic_block("end");

                let current_midi =
                    MidiValue::new(ctx.func.get_nth_param(0).unwrap().into_pointer_value());
                let push_evt =
                    MidiEventValue::new(ctx.func.get_nth_param(1).unwrap().into_pointer_value());

                let current_count = current_midi.get_count(ctx.b);
                let can_push_cond = ctx.b.build_int_compare(
                    IntPredicate::ULT,
                    current_count,
                    ctx.context
                        .i8_type()
                        .const_int(MIDI_EVENT_COUNT as u64, false),
                    "canpushcond",
                );
                ctx.b
                    .build_conditional_branch(&can_push_cond, &can_push_block, &end_block);
                ctx.b.position_at_end(&can_push_block);

                let current_event = current_midi.get_event(ctx.b, current_count);
                push_evt.copy_to(ctx.b, module, &current_event);
                let new_count = ctx.b.build_int_add(
                    current_count,
                    ctx.context.i8_type().const_int(1, false),
                    "newcount",
                );
                current_midi.set_count(ctx.b, &new_count);
                ctx.b.build_unconditional_branch(&end_block);
                ctx.b.position_at_end(&end_block);
                ctx.b.build_return(None);
            },
        );
    }
}
