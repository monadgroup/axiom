
#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash)]
pub struct SourcePos {
    pub line: usize,
    pub column: usize
}

pub const UNDEF_SOURCE_POS: SourcePos = SourcePos { line: 0, column: 0 };

#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash)]
pub struct SourceRange(pub SourcePos, pub SourcePos);

pub const UNDEF_SOURCE_RANGE: SourceRange = SourceRange(UNDEF_SOURCE_POS, UNDEF_SOURCE_POS);
