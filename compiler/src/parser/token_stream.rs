use ast::{SourcePos, SourceRange};
use parser::{Token, TokenType};
use regex::Regex;
use std::iter::Peekable;

type TokenMatcher = (Regex, TokenType);

lazy_static! {
    static ref PAIR_MATCHES: [TokenMatcher; 48] = [
        // multi-char tokens
        get_matcher(r"\.\.\.", TokenType::Ellipsis),
        get_matcher(r"==", TokenType::EqualTo),
        get_matcher(r"!=", TokenType::NotEqualTo),
        get_matcher(r"<=", TokenType::Lte),
        get_matcher(r">=", TokenType::Gte),
        get_matcher(r"\+=", TokenType::PlusAssign),
        get_matcher(r"-=", TokenType::MinusAssign),
        get_matcher(r"\*=", TokenType::TimesAssign),
        get_matcher(r"/=", TokenType::DivideAssign),
        get_matcher(r"%=", TokenType::ModuloAssign),
        get_matcher(r"\^=", TokenType::PowerAssign),
        get_matcher(r"->", TokenType::Cast),
        get_matcher(r"\+\+", TokenType::Increment),
        get_matcher(r"--", TokenType::Decrement),
        get_matcher(r"\^\^", TokenType::BitwiseXor),
        get_matcher(r"&&", TokenType::LogicalAnd),
        get_matcher(r"\|\|", TokenType::LogicalOr),
        get_matcher(r"/\*", TokenType::CommentOpen),
        get_matcher(r"\*/", TokenType::CommentClose),

        // free tokens
        get_matcher(r"'((?:[^'\\]|\\.)*)'", TokenType::SingleString),
        get_matcher(r#""((?:[^"\\]|\\.)*)""#, TokenType::DoubleString),
        get_matcher(r"([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)", TokenType::Number),
        get_matcher(r":([a-gA-G]#?[0-9]+)", TokenType::Note),
        get_matcher(r"([_a-zA-Z][_a-zA-Z0-9]*(?:\[\])?)", TokenType::Identifier),

        // single-char tokens
        get_matcher(r"\+", TokenType::Plus),
        get_matcher(r"-", TokenType::Minus),
        get_matcher(r"\*", TokenType::Times),
        get_matcher(r"/", TokenType::Divide),
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

        get_matcher("\n", TokenType::EndOfLine)
    ];
}

fn get_matcher(regex: &str, token_type: TokenType) -> TokenMatcher {
    let regex_str = format!(r"^[^\S\n]*{}[^\S\n]*", regex);
    (Regex::new(&regex_str).unwrap(), token_type)
}

struct TokenIterator<'a> {
    data: &'a str,
    cursor: usize,
    current_pos: SourcePos,
}

impl<'a> TokenIterator<'a> {
    pub fn new(data: &'a str) -> TokenIterator<'a> {
        TokenIterator {
            data,
            cursor: 0,
            current_pos: SourcePos { line: 0, column: 0 },
        }
    }
}

impl<'a> Iterator for TokenIterator<'a> {
    type Item = Token;

    fn next(&mut self) -> Option<Token> {
        if self.cursor >= self.data.len() {
            return None;
        }

        let token_start = self.current_pos;
        let remaining_data = &self.data[self.cursor..];

        let matched_pair = PAIR_MATCHES
            .iter()
            .filter_map(|matcher| {
                let maybe_caps = matcher.0.captures(remaining_data);
                match maybe_caps {
                    Some(caps) => Some((matcher.1, caps)),
                    None => None,
                }
            })
            .next();

        let token = match matched_pair {
            Some((token_type, captures)) => {
                let capture_length = captures[0].len();
                let token_end = if token_type == TokenType::EndOfLine {
                    SourcePos {
                        line: self.current_pos.line + 1,
                        column: 0,
                    }
                } else {
                    SourcePos {
                        line: self.current_pos.line,
                        column: self.current_pos.column + capture_length,
                    }
                };
                let token_content = if captures.len() > 1 { &captures[1] } else { "" };

                self.cursor += capture_length;
                self.current_pos = token_end;
                Token::new(
                    SourceRange(token_start, token_end),
                    token_type,
                    token_content.to_owned(),
                )
            }
            None => {
                // move cursor to end so the next iteration returns None
                self.cursor = self.data.len();

                Token::new(
                    SourceRange(token_start, token_start),
                    TokenType::Unknown,
                    "".to_owned(),
                )
            },
        };
        println!("Emit {:?}", &token);
        Some(token)
    }
}

pub type TokenStream<'a> = Peekable<Box<Iterator<Item = Token> + 'a>>;

pub fn get_token_stream<'a>(data: &'a str) -> TokenStream<'a> {
    let mut in_single_comment = false;
    let mut multi_comment_depth = 0;

    let boxed: Box<Iterator<Item = Token>> =
        Box::new(TokenIterator::<'a>::new(data).filter(move |token| {
            match token.token_type {
                TokenType::Hash => in_single_comment = true,
                TokenType::EndOfLine => in_single_comment = false,
                TokenType::CommentOpen => multi_comment_depth += 1,
                _ => {}
            }

            let is_valid = !in_single_comment && multi_comment_depth == 0;

            if token.token_type == TokenType::CommentClose && multi_comment_depth > 0 {
                multi_comment_depth -= 1;
            }

            is_valid
        }));

    boxed.peekable()
}
