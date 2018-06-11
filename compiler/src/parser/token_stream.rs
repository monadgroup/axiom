use parser::{Token, TokenType};
use regex::Regex;

type TokenMatcher = (Regex, TokenType);

lazy_static! {
   static ref PAIR_MATCHES: [TokenMatcher; 24] = [
        get_matcher(r"\+", TokenType::Plus),
        get_matcher(r"-", TokenType::Minus),
        get_matcher(r"\*", TokenType::Times),
        get_matcher(r"\/", TokenType::Divide),
        get_matcher(r"%", TokenType::Modulo),
        get_matcher(r"\^", TokenType::Power),
        get_matcher(r"=", TokenType::Assign),
        get_matcher(r"!", TokenType::Not),
        get_matcher(r"\(", TokenType::OpenBracket),
        get_matcher(r"\)", TokenType::CloseBracket),
        get_matcher(r"\[", TokenType::OpenSquare),
        get_matcher(r"\]", TokenType::CloseSquare),
        get_matcher(r"\{", TokenType::OpenCurly),
        get_matcher(r"\}", TokenType::CloseCurly),
        get_matcher(r",", TokenType::Comma),
        get_matcher(r";", TokenType::Semicolon),
        get_matcher(r"\.", TokenType::Dot),
        get_matcher(r":", TokenType::Colon),
        get_matcher(r"#", TokenType::Hash),
        get_matcher(r">", TokenType::Gt),
        get_matcher(r"<", TokenType::Lt),
        get_matcher(r"&", TokenType::BitwiseAnd),
        get_matcher(r"\|", TokenType::BitwiseOr),

        get_matcher(r"\n", TokenType::EndOfLine)
    ];
}

pub struct TokenStream<'a> {
    remaining_data: &'a str,
    current_line: i32,
    current_column: i32,
    is_single_line_comment: bool,
    multiline_comment_count: i32,

    peak_buffer: Option<Token>
}

fn get_matcher(regex: &str, token_type: TokenType) -> TokenMatcher {
    let regex_str = format!(r"[^\S\n]*{}[^\S\n]*", regex);
    (Regex::new(&regex_str).unwrap(), token_type)
}
