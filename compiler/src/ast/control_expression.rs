use ast::{Assignable, ControlType, Expression, SourceRange};

#[derive(Debug)]
pub struct ControlExpression {
    pos: SourceRange,
    name: String,
    control_type: ControlType,
    prop: String,
}

impl ControlExpression {
    pub fn new(
        pos: SourceRange,
        name: String,
        control_type: ControlType,
        prop: String,
    ) -> ControlExpression {
        ControlExpression {
            pos,
            name,
            control_type,
            prop,
        }
    }

    pub fn name(&self) -> &str {
        &self.name
    }
    pub fn control_type(&self) -> ControlType {
        self.control_type
    }
    pub fn prop(&self) -> &str {
        &self.prop
    }
}

impl Expression for ControlExpression {
    fn assignables(&self) -> Result<Vec<Assignable>, SourceRange> {
        Ok(vec![Assignable::Control {
            pos: self.pos,
            name: self.name.to_owned(),
            control_type: self.control_type,
            prop: self.prop.to_owned(),
        }])
    }
    fn pos(&self) -> &SourceRange {
        &self.pos
    }
}
