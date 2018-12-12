use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use crate::ast::{ControlField, ControlType, ScopeField};

pub struct ScopeControl;
impl Control for ScopeControl {
    fn control_type() -> ControlType {
        ControlType::Scope
    }

    fn gen_fields(generator: &ControlFieldGenerator) {
        generator.generate(
            ControlField::Scope(ScopeField::Value),
            &default_copy_getter,
            &default_copy_setter,
        );
    }
}
