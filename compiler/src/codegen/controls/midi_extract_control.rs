use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use ast::{ControlField, ControlType, MidiExtractField};

pub struct MidiExtractControl;
impl Control for MidiExtractControl {
    fn control_type() -> ControlType {
        ControlType::MidiExtract
    }

    fn gen_fields(generator: &ControlFieldGenerator) {
        generator.generate(
            ControlField::MidiExtract(MidiExtractField::Value),
            &default_copy_getter,
            &default_copy_setter,
        );
    }
}
