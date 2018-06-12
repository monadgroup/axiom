
#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash)]
pub struct SourcePos {
    pub line: usize,
    pub column: usize
}

#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash)]
pub struct SourceRange(pub SourcePos, pub SourcePos);
