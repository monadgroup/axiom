use std::fmt;

#[derive(Eq, PartialEq, Clone, Copy, Hash)]
pub struct SourcePos {
    pub line: usize,
    pub column: usize,
}

impl fmt::Debug for SourcePos {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}:{}", self.line, self.column)
    }
}

pub const UNDEF_SOURCE_POS: SourcePos = SourcePos { line: 0, column: 0 };

#[derive(Eq, PartialEq, Clone, Copy, Hash)]
pub struct SourceRange(pub SourcePos, pub SourcePos);

impl fmt::Debug for SourceRange {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{:?} -> {:?}", self.0, self.1)
    }
}

pub const UNDEF_SOURCE_RANGE: SourceRange = SourceRange(UNDEF_SOURCE_POS, UNDEF_SOURCE_POS);
