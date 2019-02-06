use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use crate::ast::{ControlField, ControlType, MidiField};

pub struct MidiControl;
impl Control for MidiControl {
    fn control_type() -> ControlType {
        ControlType::Midi
    }

    fn gen_fields(generator: &ControlFieldGenerator) {
        generator.generate(
            ControlField::Midi(MidiField::Value),
            &default_copy_getter,
            &default_copy_setter,
        );
    }
}
