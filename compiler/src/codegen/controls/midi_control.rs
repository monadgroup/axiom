use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use ast::{ControlField, ControlType, MidiField};

pub struct MidiControl;
impl Control for MidiControl {
    fn control_type() -> ControlType {
        ControlType::Audio
    }

    fn gen_fields(generator: &ControlFieldGenerator) {
        generator.generate(
            ControlField::Midi(MidiField::Value),
            &default_copy_getter,
            &default_copy_setter,
        );
    }
}
