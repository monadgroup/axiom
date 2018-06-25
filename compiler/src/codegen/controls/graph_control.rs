use super::ControlContext;
use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use ast::{ControlField, ControlType, GraphField};
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

fn speed_field_getter(_control: &mut ControlContext, _out_val: PointerValue) {
    // todo
}

fn speed_field_setter(_control: &mut ControlContext, _in_val: PointerValue) {
    // todo
}
