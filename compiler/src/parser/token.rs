use ast::SourceRange;

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

    Unknown,
}

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Token {
    pub pos: SourceRange,
    pub token_type: TokenType,
    pub content: String,
}

impl Token {
    pub fn new(pos: SourceRange, token_type: TokenType, content: String) -> Token {
        Token {
            pos,
            token_type,
            content,
        }
    }
}
