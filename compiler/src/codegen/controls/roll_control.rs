use super::ControlContext;
use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use crate::ast::{ControlField, ControlType, RollField};
use inkwell::values::PointerValue;

pub struct RollControl;
impl Control for RollControl {
    fn control_type() -> ControlType {
        ControlType::Roll
    }

    fn gen_fields(generator: &ControlFieldGenerator) {
        generator.generate(
            ControlField::Roll(RollField::Value),
            &default_copy_getter,
            &default_copy_setter,
        );
        generator.generate(
            ControlField::Roll(RollField::Speed),
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
