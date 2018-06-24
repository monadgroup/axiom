use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use ast::{ControlField, ControlType, GraphField};
use codegen::ControlContext;
use inkwell::values::PointerValue;

pub struct GraphControl;
impl Control for GraphControl {
    fn control_type() -> ControlType {
        ControlType::Graph
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

fn speed_field_getter(control: &mut ControlContext, out_val: PointerValue) {
    // todo
}

fn speed_field_setter(control: &mut ControlContext, in_val: PointerValue) {
    // todo
}
