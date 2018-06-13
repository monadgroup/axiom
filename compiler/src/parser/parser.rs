use {CompileError, CompileResult};
use ast::{Block, CastExpression, ControlExpression, Expression, Form, FormType, NoteExpression, NumberExpression, SourcePos, SourceRange, UnaryExpression, UnaryOperation, VariableExpression};
use parser::{get_token_stream, Token, TokenStream, TokenType};
use regex::Regex;
use std::iter::{Peekable, repeat};

const PRECEDENCE_NONE: i32 = 0;

const PRECEDENCE_CASTING: i32 = 1;
const PRECEDENCE_UNARY: i32 = 2;

const PRECEDENCE_POWER: i32 = 3;

const PRECEDENCE_BITWISE: i32 = 4;

const PRECEDENCE_MULTIPLY: i32 = 5;
const PRECEDENCE_DIVIDE: i32 = 5;
const PRECEDENCE_MODULO: i32 = 5;

const PRECEDENCE_ADD: i32 = 6;
const PRECEDENCE_SUBTRACT: i32 = 6;

const PRECEDENCE_EQUALITY: i32 = 7;
const PRECEDENCE_LOGICAL: i32 = 8;

const PRECEDENCE_ASSIGNMENT: i32 = 10;

const PRECEDENCE_ALL: i32 = 11;

enum ParsedExpr {
    Continue(Box<Expression>),
    End(Box<Expression>),
}

pub struct Parser {
    content: String
}

type ExprResult = CompileResult<Box<Expression>>;

impl Parser {
    pub fn new(content: String) -> Parser {
        Parser {
            content
        }
    }

    pub fn parse(&self) -> CompileResult<Block> {
        let mut stream = get_token_stream(&self.content);
        let mut expressions = Vec::new();

        // todo: this is probably really bad and can be made much nicer
        loop {
            let peek_token = stream.peek().cloned();
            match peek_token {
                Some(Token { token_type: TokenType::EndOfLine, .. }) => {}
                Some(_) => {
                    match Parser::parse_expression(&mut stream, PRECEDENCE_ALL) {
                        Ok(expr) => expressions.push(expr),
                        Err(err) => return Err(err)
                    }

                    // ensure next token is a newline or end of stream
                    let next_token = stream.next();
                    match next_token {
                        Some(Token { token_type: TokenType::EndOfLine, .. }) => {}
                        Some(token) => return Err(CompileError::mismatched_token(TokenType::EndOfLine, token)),
                        None => break
                    }
                }
                None => break
            }
        }

        Ok(Block::new(expressions))
    }

    fn parse_expression(stream: &mut TokenStream, precedence: i32) -> ExprResult {
        // todo: there's probably a nice iterator-based solution for this
        let mut result = match Parser::parse_prefix(stream, precedence) {
            Ok(prefix) => prefix,
            Err(err) => return Err(err)
        };
        loop {
            result = match Parser::parse_postfix(stream, result, precedence) {
                Ok(ParsedExpr::Continue(expr)) => expr,
                Ok(ParsedExpr::End(expr)) => return Ok(expr),
                Err(err) => return Err(err)
            }
        }
    }

    fn parse_form(stream: &mut TokenStream) -> CompileResult<Form> {
        // ensure it starts with an open square bracket
        let start_pos = match Parser::expect_token(TokenType::OpenSquare, stream.next()) {
            Ok(token) => token.pos.0,
            Err(err) => return Err(err)
        };

        let (form_name, name_pos) = match Parser::expect_token(TokenType::Identifier, stream.next()) {
            Ok(token) => (token.content, token.pos),
            Err(err) => return Err(err)
        };

        let form_type = match form_name.as_ref() {
            "none" => FormType::None,
            "control" => FormType::Control,
            "osc" => FormType::Oscillator,
            "note" => FormType::Note,
            "freq" => FormType::Frequency,
            "beats" => FormType::Beats,
            "secs" => FormType::Seconds,
            "samples" => FormType::Samples,
            "db" => FormType::Db,
            "amp" => FormType::Amplitude,
            "q" => FormType::Q,
            _ => return Err(CompileError::unknown_form(form_name, name_pos))
        };

        let end_pos = match Parser::expect_token(TokenType::CloseSquare, stream.next()) {
            Ok(token) => token.pos.1,
            Err(err) => return Err(err)
        };

        Ok(Form::new(SourceRange(start_pos, end_pos), form_type))
    }

    fn parse_prefix(stream: &mut TokenStream, precedence: i32) -> ExprResult {
        let first_token = stream.peek().cloned();

        match first_token {
            Some(Token { token_type: TokenType::Colon, ref pos, .. }) => Parser::parse_control_expr(stream, "".to_owned(), pos.0),
            Some(Token { token_type: TokenType::OpenSquare, .. }) => Parser::parse_open_square_token_expr(stream),
            Some(Token { token_type: TokenType::Note, .. }) => Parser::parse_note_token_expr(stream),
            Some(Token { token_type: TokenType::Number, .. }) => Parser::parse_number_token_expr(stream),
            Some(Token { token_type: TokenType::DoubleString, content, pos, .. }) => Parser::parse_control_expr(stream, content, pos.0),
            Some(Token { token_type: TokenType::Identifier, .. }) => Parser::parse_identifier_token_expr(stream),
            Some(Token { token_type: TokenType::OpenBracket, .. }) => Parser::parse_open_bracket_token_expr(stream),
            Some(Token { token_type: TokenType::Plus, .. })
            | Some(Token { token_type: TokenType::Minus, .. })
            | Some(Token { token_type: TokenType::Not, .. })
            | Some(Token { token_type: TokenType::Increment, .. })
            | Some(Token { token_type: TokenType::Decrement, .. }) => Parser::parse_unary_token_expr(stream),
            Some(token) => Err(CompileError::unexpected_token(token)),
            None => Err(CompileError::UnexpectedEnd)
        }
    }

    fn parse_postfix(stream: &mut TokenStream, prefix: Box<Expression>, precedence: i32) -> CompileResult<ParsedExpr> {
        unimplemented!();
    }

    fn parse_open_square_token_expr(stream: &mut TokenStream) -> ExprResult {
        let form = match Parser::parse_form(stream) {
            Ok(form) => form,
            Err(err) => return Err(err)
        };
        let expr = match Parser::parse_expression(stream, PRECEDENCE_UNARY) {
            Ok(expr) => expr,
            Err(err) => return Err(err)
        };

        let form_start = form.pos().0.clone();
        let expr_end = expr.pos().1.clone();

        Ok(Box::new(CastExpression::new(SourceRange(form_start, expr_end), form, expr, true)))
    }

    fn parse_note_token_expr(stream: &mut TokenStream) -> ExprResult {
        static NOTE_NAMES: [&str; 12] = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
        lazy_static! {
            static ref NOTE_REGEX: Regex = Regex::new(r"([a-gA-G]#?)([0-9]+)").unwrap();
        }

        let note_token = match Parser::expect_token(TokenType::Note, stream.next()) {
            Ok(token) => token,
            Err(err) => return Err(err)
        };

        let captures = NOTE_REGEX.captures(&note_token.content).unwrap();
        let note_name = captures[1].to_uppercase();
        let note_num = match NOTE_NAMES.iter().position(|&s| s == note_name) {
            Some(index) => index,
            None => return Err(CompileError::unknown_note(note_name, note_token.pos))
        };
        let octave = captures[2].parse::<usize>().unwrap();
        let midi_num = note_num + octave * NOTE_NAMES.len();
        return Ok(Box::new(NoteExpression::new(note_token.pos, midi_num as i32)));
    }

    fn parse_number_token_expr(stream: &mut TokenStream) -> ExprResult {
        let num_token = match Parser::expect_token(TokenType::Number, stream.next()) {
            Ok(token) => token,
            Err(err) => return Err(err)
        };

        let base_num_val = num_token.content.parse::<f32>().unwrap();

        // Attempt to find a postfix after the number to set a magnitude or form.
        // Postfixes are always in the following format: 0.5[magnitude][form], where both
        // magnitude and form are optional. Magnitude is always a single character, form is one
        // or two.
        // If we can't match either of these in the next identifier token, we assume it's a token
        // for something later on and ignore it.
        let (matched_token, num_val, val_form) = match stream.peek() {
            Some(Token { token_type: TokenType::Identifier, pos, content, .. }) => {
                let post_mul_text = content.to_uppercase();

                let (matched_mul, adjusted_val) = {
                    if post_mul_text.starts_with("K") { (true, base_num_val * 1e3) }
                    else if post_mul_text.starts_with("M") { (true, base_num_val * 1e6) }
                    else if post_mul_text.starts_with("G") { (true, base_num_val * 1e12) }
                    else if post_mul_text.starts_with("T") { (true, base_num_val * 1e15) }
                    else { (false, base_num_val) }
                };

                let remaining_content = if matched_mul { &post_mul_text[1..] } else { post_mul_text.as_ref() };
                let (matched_form, new_form) = {
                    if remaining_content == "HZ" { (true, FormType::Frequency) }
                    else if remaining_content == "DB" { (true, FormType::Db) }
                    else if remaining_content == "Q" { (true, FormType::Q) }
                    else if remaining_content == "S" { (true, FormType::Seconds) }
                    else if remaining_content == "B" { (true, FormType::Beats) }
                    else { (false, FormType::None) }
                };

                if matched_form || (matched_mul && post_mul_text.len() == 1) {
                    (true, adjusted_val, Form::new(pos.clone(), new_form))
                } else {
                    (false, base_num_val, Form::new(pos.clone(), FormType::None))
                }
            }
            _ => (false, base_num_val, Form::new(num_token.pos.clone(), FormType::None))
        };

        // consume the identifier if it matched (needs to happen out here since `stream` is borrowed
        // by the peek in the match
        if matched_token {
            stream.next();
        }

        Ok(Box::new(NumberExpression::new(SourceRange(num_token.pos.0, val_form.pos().1), num_val, val_form)))
    }

    fn parse_unary_token_expr(stream: &mut TokenStream) -> ExprResult {
        let (operator, pos) = match stream.next() {
            Some(Token { token_type: TokenType::Plus, pos, .. }) => (UnaryOperation::Positive, pos),
            Some(Token { token_type: TokenType::Minus, pos, .. }) => (UnaryOperation::Negative, pos),
            Some(Token { token_type: TokenType::Not, pos, .. }) => (UnaryOperation::Not, pos),
            Some(token) => return Err(CompileError::unexpected_token(token)),
            None => return Err(CompileError::UnexpectedEnd)
        };

        let expr = match Parser::parse_expression(stream, PRECEDENCE_UNARY) {
            Ok(expr) => expr,
            Err(err) => return Err(err)
        };
        Ok(Box::new(UnaryExpression::new(SourceRange(pos.0, expr.pos().1), operator, expr)))
    }

    fn parse_identifier_token_expr(stream: &mut TokenStream) -> ExprResult {
        let identifier_token = match Parser::expect_token(TokenType::Identifier, stream.next()) {
            Ok(token) => token,
            Err(err) => return Err(err)
        };

        match stream.peek().cloned() {
            Some(Token { token_type: TokenType::OpenBracket, content, pos }) => Parser::parse_call_expr(stream, content.to_owned(), pos.0),
            Some(Token { token_type: TokenType::Colon, content, pos }) => Parser::parse_control_expr(stream, content.to_owned(), pos.0),
            _ => Ok(Box::new(VariableExpression::new(identifier_token.pos, identifier_token.content)))
        }
    }

    fn parse_open_bracket_token_expr(stream: &mut TokenStream) -> ExprResult {
        unimplemented!();
    }

    fn parse_control_expr(stream: &mut TokenStream, name: String, start_pos: SourcePos) -> ExprResult {
        unimplemented!();
    }

    fn parse_call_expr(stream: &mut TokenStream, name: String, start_pos: SourcePos) -> ExprResult {
        unimplemented!();
    }

    fn expect_token(expected_type: TokenType, token_stream: Option<Token>) -> CompileResult<Token> {
        match token_stream {
            Some(token) => match token.token_type {
                token_type if token_type == expected_type => Ok(token),
                _ => Err(CompileError::mismatched_token(expected_type, token))
            },
            None => Err(CompileError::UnexpectedEnd)
        }
    }
}
