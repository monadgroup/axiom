use crate::ast::ControlType;
use std::fmt;

#[derive(Debug, Clone)]
pub struct Control {
    pub name: String,
    pub control_type: ControlType,
    pub value_written: bool,
    pub value_read: bool,
}

impl Control {
    pub fn new(
        name: String,
        control_type: ControlType,
        value_written: bool,
        value_read: bool,
    ) -> Control {
        Control {
            name,
            control_type,
            value_written,
            value_read,
        }
    }
}

impl fmt::Display for Control {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{:?} \"{}\" [", self.control_type, self.name)?;
        if self.value_written {
            write!(f, "written")?;
        }
        if self.value_written && self.value_read {
            write!(f, " ")?;
        }
        if self.value_read {
            write!(f, "read")?;
        }
        write!(f, "]")
    }
}
