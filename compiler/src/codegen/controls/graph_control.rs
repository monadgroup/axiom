use super::ControlContext;
use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use ast::{ControlField, ControlType, GraphField};
use inkwell::context::Context;
use inkwell::types::StructType;
use inkwell::values::PointerValue;

pub struct GraphControl;
impl Control for GraphControl {
    fn control_type() -> ControlType {
        ControlType::Graph
    }

    fn data_type(context: &Context) -> StructType {
        context.struct_type(
            &[
                &context.i8_type(),                // curve count
                &context.f32_type().array_type(9), // start values
                &context.f32_type().array_type(8), // end positions
                &context.f32_type().array_type(8), // tension
                &context.i8_type().array_type(8),  // states
            ],
            false,
        )
    }

    fn gen_fields(generator: &ControlFieldGenerator) {
        generator.generate(
            ControlField::Graph(GraphField::Value),
            &default_copy_getter,
            &default_copy_setter,
        );
        generator.generate(
            ControlField::Graph(GraphField::Speed),
            &speed_field_getter,
            &speed_field_setter,
        );
    }
}

fn speed_field_getter(_control: &mut ControlContext, _out_val: PointerValue) {
    // todo
}

fn speed_field_setter(_control: &mut ControlContext, _in_val: PointerValue) {
    // todo
}
