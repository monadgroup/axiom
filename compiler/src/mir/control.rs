#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum ControlType {
    Audio,
    Graph,
    Midi,
    Roll,
    Scope,
    NumExtract,
    MidiExtract
}

// todo: control needs to store Option<value>

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub struct Control {
    control_type: ControlType,
    group_id: usize,
    value_written: bool,
    value_read: bool
}

impl Control {
    pub fn new(control_type: ControlType, group_id: usize, value_written: bool, value_read: bool) -> Control {
        Control {
            control_type,
            group_id,
            value_written,
            value_read
        }
    }

    pub fn get_control_type(&self) -> ControlType {
        self.control_type
    }

    pub fn get_group_id(&self) -> usize {
        self.group_id
    }

    pub fn get_value_written(&self) -> bool {
        self.value_written
    }

    pub fn get_value_read(&self) -> bool {
        self.value_read
    }
}
