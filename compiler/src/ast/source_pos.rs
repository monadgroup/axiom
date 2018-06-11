
#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash)]
pub struct SourcePos {
    line: i32,
    column: i32
}

impl SourcePos {
    pub fn new(line: i32, column: i32) -> SourcePos {
        SourcePos {
            line,
            column
        }
    }

    pub fn get_line(&self) -> i32 {
        self.line
    }

    pub fn get_column(&self) -> i32 {
        self.column
    }
}
