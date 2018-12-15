#[derive(Debug, PartialEq, Eq, Clone, Hash)]
pub struct ValueSocket {
    pub group_id: usize,
    pub value_written: bool,
    pub value_read: bool,
    pub is_extractor: bool,
}

impl ValueSocket {
    pub fn new(group_id: usize, value_written: bool, value_read: bool, is_extractor: bool) -> Self {
        ValueSocket {
            group_id,
            value_written,
            value_read,
            is_extractor,
        }
    }
}
