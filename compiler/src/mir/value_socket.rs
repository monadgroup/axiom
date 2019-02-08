use std::fmt;

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

impl fmt::Display for ValueSocket {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "%{}", self.group_id)?;

        let mut tags = Vec::new();
        if self.value_written {
            tags.push("written");
        }
        if self.value_read {
            tags.push("read");
        }
        if self.is_extractor {
            tags.push("extractor");
        }

        if !tags.is_empty() {
            write!(f, " [")?;
            for (tag_index, tag) in tags.iter().enumerate() {
                write!(f, "{}", tag)?;

                if tag_index != tags.len() - 1 {
                    write!(f, ", ")?;
                }
            }
            write!(f, "]")?;
        }

        Ok(())
    }
}
