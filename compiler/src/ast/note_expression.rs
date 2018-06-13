use ast::{SourceRange, Expression};

#[derive(Debug)]
pub struct NoteExpression {
    pos: SourceRange,
    note: i32
}

impl NoteExpression {
    pub fn new(pos: SourceRange, note: i32) -> NoteExpression {
        NoteExpression {
            pos,
            note
        }
    }

    pub fn note(&self) -> i32 { self.note }
}

impl Expression for NoteExpression {
    fn pos(&self) -> &SourceRange { &self.pos }
}
