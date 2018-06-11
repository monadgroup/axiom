use ast::{SourcePos, Expression};

#[derive(Debug)]
pub struct NoteExpression {
    start_pos: SourcePos,
    end_pos: SourcePos,
    note: i32
}

impl NoteExpression {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, note: i32) -> NoteExpression {
        NoteExpression {
            start_pos,
            end_pos,
            note
        }
    }

    pub fn get_note(&self) -> i32 {
        self.note
    }
}

impl Expression for NoteExpression {
    fn get_start_pos(&self) -> SourcePos { self.start_pos }
    fn get_end_pos(&self) -> SourcePos { self.end_pos }
}
