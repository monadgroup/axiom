use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use ast::{AudioExtractField, ControlField, ControlType};

pub struct AudioExtractControl;
impl Control for AudioExtractControl {
    fn control_type() -> ControlType {
        ControlType::AudioExtract
    }

    fn gen_fields(generator: &ControlFieldGenerator) {
        generator.generate(
            ControlField::AudioExtract(AudioExtractField::Value),
            &default_copy_getter,
            &default_copy_setter,
        );
    }
}
