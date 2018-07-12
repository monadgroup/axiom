use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use ast::{AudioField, ControlField, ControlType};

pub struct AudioControl;
impl Control for AudioControl {
    fn control_type() -> ControlType {
        ControlType::Audio
    }

    fn gen_fields(generator: &ControlFieldGenerator) {
        generator.generate(
            ControlField::Audio(AudioField::Value),
            &default_copy_getter,
            &default_copy_setter,
        );
    }
}
