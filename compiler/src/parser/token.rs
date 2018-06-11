use ast::{SourcePos};

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum TokenType {
    // single-char tokens
    Plus,
    Minus,
    Times,
    Divide,
    Modulo,
    Power,
    Assign,
    Not,
    OpenBracket,
    CloseBracket,
    OpenSquare,
    CloseSquare,
    OpenCurly,
    CloseCurly,
    Comma,
    Semicolon,
    Dot,
    Colon,
    Hash,
    Gt,
    Lt,
    BitwiseAnd,
    BitwiseOr,
    EndOfLine,
    EndOfFile,

    // multi-char tokens
    Gte,
    Lte,
    EqualTo,
    NotEqualTo,
    Ellipsis,
    Cast,
    CommentOpen,
    CommentClose,
    PlusAssign,
    MinusAssign,
    TimesAssign,
    DivideAssign,
    ModuloAssign,
    PowerAssign,
    Increment,
    Decrement,
    BitwiseXor,
    LogicalAnd,
    LogicalOr,

    // free tokens
    SingleString,
    DoubleString,
    Number,
    Note,
    Identifier,

    Unknown
}

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Token {
    start_pos: SourcePos,
    end_pos: SourcePos,
    token_type: TokenType,
    content: String
}

impl Token {
    pub fn new(start_pos: SourcePos, end_pos: SourcePos, token_type: TokenType, content: String) -> Token {
        Token {
            start_pos,
            end_pos,
            token_type,
            content
        }
    }

    pub fn get_start_pos(&self) -> SourcePos {
        self.start_pos
    }

    pub fn get_end_pos(&self) -> SourcePos {
        self.end_pos
    }

    pub fn get_token_type(&self) -> TokenType {
        self.token_type
    }

    pub fn get_content(&self) -> &str {
        &self.content
    }
}
