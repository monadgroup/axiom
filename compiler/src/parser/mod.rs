mod token;
mod token_stream;

pub use self::token::{Token, TokenType};
pub use self::token_stream::{get_token_stream, TokenStream};

use ast::{AssignExpression, Block, CallExpression, CastExpression, ControlExpression, ControlType,
          Expression, Form, FormType, LValueExpression, MathExpression, NoteExpression,
          NumberExpression, OperatorType, PostfixExpression, PostfixOperation, SourcePos,
          SourceRange, TupleExpression, UnaryExpression, UnaryOperation, VariableExpression};
use regex::Regex;
use {CompileError, CompileResult};

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

impl ParsedExpr {
    pub fn continue_expr(result: ExprResult) -> CompileResult<ParsedExpr> {
        match result {
            Ok(expr) => Ok(ParsedExpr::Continue(expr)),
            Err(err) => Err(err),
        }
    }
}

pub struct Parser<'a> {
    content: &'a str,
}

type ExprResult = CompileResult<Box<Expression>>;

impl<'a> Parser<'a> {
    pub fn new(content: &'a str) -> Parser<'a> {
        Parser { content }
    }

    pub fn parse(&self) -> CompileResult<Block> {
        let mut stream = get_token_stream(self.content);
        let mut expressions = Vec::new();

        // todo: this is probably really bad and can be made much nicer
        loop {
            let peek_token = stream.peek().cloned();
            match peek_token {
                Some(Token {
                    token_type: TokenType::EndOfLine,
                    ..
                }) => {}
                Some(_) => {
                    match Parser::parse_expression(&mut stream, PRECEDENCE_ALL) {
                        Ok(expr) => expressions.push(expr),
                        Err(err) => return Err(err),
                    }

                    // ensure next token is a newline or end of stream
                    let next_token = stream.next();
                    match next_token {
                        Some(Token {
                            token_type: TokenType::EndOfLine,
                            ..
                        }) => {}
                        Some(token) => {
                            return Err(CompileError::mismatched_token(TokenType::EndOfLine, token))
                        }
                        None => break,
                    }
                }
                None => break,
            }
        }

        Ok(Block::new(expressions))
    }

    fn parse_expression(stream: &mut TokenStream, precedence: i32) -> ExprResult {
        // todo: there's probably a nice iterator-based solution for this
        let mut result = match Parser::parse_prefix(stream) {
            Ok(prefix) => prefix,
            Err(err) => return Err(err),
        };
        loop {
            result = match Parser::parse_postfix(stream, result, precedence) {
                Ok(ParsedExpr::Continue(expr)) => expr,
                Ok(ParsedExpr::End(expr)) => return Ok(expr),
                Err(err) => return Err(err),
            }
        }
    }

    fn parse_form(stream: &mut TokenStream) -> CompileResult<Form> {
        // ensure it starts with an open square bracket
        let start_pos = match Parser::expect_token(TokenType::OpenSquare, stream.next()) {
            Ok(token) => token.pos.0,
            Err(err) => return Err(err),
        };

        let (form_name, name_pos) = match Parser::expect_token(TokenType::Identifier, stream.next())
        {
            Ok(token) => (token.content, token.pos),
            Err(err) => return Err(err),
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
            _ => return Err(CompileError::unknown_form(form_name, name_pos)),
        };

        let end_pos = match Parser::expect_token(TokenType::CloseSquare, stream.next()) {
            Ok(token) => token.pos.1,
            Err(err) => return Err(err),
        };

        Ok(Form::new(SourceRange(start_pos, end_pos), form_type))
    }

    fn parse_prefix(stream: &mut TokenStream) -> ExprResult {
        let first_token = stream.peek().cloned();

        match first_token {
            Some(Token {
                token_type: TokenType::Colon,
                ref pos,
                ..
            }) => Parser::parse_control_expr(stream, "".to_owned(), pos.0),
            Some(Token {
                token_type: TokenType::OpenSquare,
                ..
            }) => Parser::parse_open_square_token_expr(stream),
            Some(Token {
                token_type: TokenType::Note,
                ..
            }) => Parser::parse_note_token_expr(stream),
            Some(Token {
                token_type: TokenType::Number,
                ..
            }) => Parser::parse_number_token_expr(stream),
            Some(Token {
                token_type: TokenType::DoubleString,
                content,
                pos,
                ..
            }) => Parser::parse_control_expr(stream, content, pos.0),
            Some(Token {
                token_type: TokenType::Identifier,
                ..
            }) => Parser::parse_identifier_token_expr(stream),
            Some(Token {
                token_type: TokenType::OpenBracket,
                ..
            }) => Parser::parse_open_bracket_token_expr(stream),
            Some(Token {
                token_type: TokenType::Plus,
                ..
            })
            | Some(Token {
                token_type: TokenType::Minus,
                ..
            })
            | Some(Token {
                token_type: TokenType::Not,
                ..
            })
            | Some(Token {
                token_type: TokenType::Increment,
                ..
            })
            | Some(Token {
                token_type: TokenType::Decrement,
                ..
            }) => Parser::parse_unary_token_expr(stream),
            Some(token) => Err(CompileError::unexpected_token(token)),
            None => Err(CompileError::UnexpectedEnd),
        }
    }

    fn parse_postfix(
        stream: &mut TokenStream,
        prefix: Box<Expression>,
        precedence: i32,
    ) -> CompileResult<ParsedExpr> {
        let token_type = match stream.peek() {
            Some(token) => token.token_type,
            None => return Err(CompileError::UnexpectedEnd),
        };

        // if this token has a lower precedence than provided, end this expression
        if Parser::get_operator_precedence(token_type) <= precedence {
            return Ok(ParsedExpr::End(prefix));
        }

        match token_type {
            TokenType::Cast => ParsedExpr::continue_expr(Parser::parse_cast_expr(stream, prefix)),
            TokenType::Increment | TokenType::Decrement => {
                ParsedExpr::continue_expr(Parser::parse_postfix_expr(stream, prefix.as_ref()))
            }
            TokenType::BitwiseAnd
            | TokenType::BitwiseOr
            | TokenType::BitwiseXor
            | TokenType::LogicalAnd
            | TokenType::LogicalOr
            | TokenType::EqualTo
            | TokenType::NotEqualTo
            | TokenType::Lt
            | TokenType::Gt
            | TokenType::Lte
            | TokenType::Gte
            | TokenType::Plus
            | TokenType::Minus
            | TokenType::Times
            | TokenType::Divide
            | TokenType::Modulo
            | TokenType::Power => {
                ParsedExpr::continue_expr(Parser::parse_math_expr(stream, prefix))
            }
            TokenType::Assign
            | TokenType::PlusAssign
            | TokenType::MinusAssign
            | TokenType::TimesAssign
            | TokenType::DivideAssign
            | TokenType::ModuloAssign
            | TokenType::PowerAssign => {
                ParsedExpr::continue_expr(Parser::parse_assign_expr(stream, prefix.as_ref()))
            }
            _ => Ok(ParsedExpr::End(prefix)),
        }
    }

    fn parse_open_square_token_expr(stream: &mut TokenStream) -> ExprResult {
        let form = match Parser::parse_form(stream) {
            Ok(form) => form,
            Err(err) => return Err(err),
        };
        let expr = match Parser::parse_expression(stream, PRECEDENCE_UNARY) {
            Ok(expr) => expr,
            Err(err) => return Err(err),
        };

        let form_start = form.pos().0;
        let expr_end = expr.pos().1;

        Ok(Box::new(CastExpression::new(
            SourceRange(form_start, expr_end),
            form,
            expr,
            true,
        )))
    }

    fn parse_note_token_expr(stream: &mut TokenStream) -> ExprResult {
        static NOTE_NAMES: [&str; 12] = [
            "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        ];
        lazy_static! {
            static ref NOTE_REGEX: Regex = Regex::new(r"([a-gA-G]#?)([0-9]+)").unwrap();
        }

        let note_token = match Parser::expect_token(TokenType::Note, stream.next()) {
            Ok(token) => token,
            Err(err) => return Err(err),
        };

        let captures = NOTE_REGEX.captures(&note_token.content).unwrap();
        let note_name = captures[1].to_uppercase();
        let note_num = match NOTE_NAMES.iter().position(|&s| s == note_name) {
            Some(index) => index,
            None => return Err(CompileError::unknown_note(note_name, note_token.pos)),
        };
        let octave = captures[2].parse::<usize>().unwrap();
        let midi_num = note_num + octave * NOTE_NAMES.len();
        Ok(Box::new(NoteExpression::new(
            note_token.pos,
            midi_num as i32,
        )))
    }

    fn parse_number_token_expr(stream: &mut TokenStream) -> ExprResult {
        let num_token = match Parser::expect_token(TokenType::Number, stream.next()) {
            Ok(token) => token,
            Err(err) => return Err(err),
        };

        let base_num_val = num_token.content.parse::<f32>().unwrap();

        // Attempt to find a postfix after the number to set a magnitude or form.
        // Postfixes are always in the following format: 0.5[magnitude][form], where both
        // magnitude and form are optional. Magnitude is always a single character, form is one
        // or two.
        // If we can't match either of these in the next identifier token, we assume it's a token
        // for something later on and ignore it.
        let (matched_token, num_val, val_form) = match stream.peek() {
            Some(Token {
                token_type: TokenType::Identifier,
                pos,
                content,
                ..
            }) => {
                let post_mul_text = content.to_uppercase();

                let (matched_mul, adjusted_val) = {
                    if post_mul_text.starts_with('K') {
                        (true, base_num_val * 1e3)
                    } else if post_mul_text.starts_with('M') {
                        (true, base_num_val * 1e6)
                    } else if post_mul_text.starts_with('G') {
                        (true, base_num_val * 1e12)
                    } else if post_mul_text.starts_with('T') {
                        (true, base_num_val * 1e15)
                    } else {
                        (false, base_num_val)
                    }
                };

                let remaining_content = if matched_mul {
                    &post_mul_text[1..]
                } else {
                    post_mul_text.as_ref()
                };
                let (matched_form, new_form) = match remaining_content {
                    "HZ" => (true, FormType::Frequency),
                    "DB" => (true, FormType::Db),
                    "Q" => (true, FormType::Q),
                    "S" => (true, FormType::Seconds),
                    "B" => (true, FormType::Beats),
                    _ => (false, FormType::None),
                };

                if matched_form || (matched_mul && post_mul_text.len() == 1) {
                    (true, adjusted_val, Form::new(*pos, new_form))
                } else {
                    (false, base_num_val, Form::new(*pos, FormType::None))
                }
            }
            _ => (
                false,
                base_num_val,
                Form::new(num_token.pos, FormType::None),
            ),
        };

        // consume the identifier if it matched (needs to happen out here since `stream` is borrowed
        // by the peek in the match
        if matched_token {
            stream.next();
        }

        Ok(Box::new(NumberExpression::new(
            SourceRange(num_token.pos.0, val_form.pos().1),
            num_val,
            val_form,
        )))
    }

    fn parse_unary_token_expr(stream: &mut TokenStream) -> ExprResult {
        let (operator, pos) = match stream.next() {
            Some(Token {
                token_type: TokenType::Plus,
                pos,
                ..
            }) => (UnaryOperation::Positive, pos),
            Some(Token {
                token_type: TokenType::Minus,
                pos,
                ..
            }) => (UnaryOperation::Negative, pos),
            Some(Token {
                token_type: TokenType::Not,
                pos,
                ..
            }) => (UnaryOperation::Not, pos),
            Some(token) => return Err(CompileError::unexpected_token(token)),
            None => return Err(CompileError::UnexpectedEnd),
        };

        let expr = match Parser::parse_expression(stream, PRECEDENCE_UNARY) {
            Ok(expr) => expr,
            Err(err) => return Err(err),
        };
        Ok(Box::new(UnaryExpression::new(
            SourceRange(pos.0, expr.pos().1),
            operator,
            expr,
        )))
    }

    fn parse_identifier_token_expr(stream: &mut TokenStream) -> ExprResult {
        let identifier_token = match Parser::expect_token(TokenType::Identifier, stream.next()) {
            Ok(token) => token,
            Err(err) => return Err(err),
        };

        match stream.peek().cloned() {
            Some(Token {
                token_type: TokenType::OpenBracket,
                content,
                pos,
            }) => Parser::parse_call_expr(stream, content.to_owned(), pos.0),
            Some(Token {
                token_type: TokenType::Colon,
                content,
                pos,
            }) => Parser::parse_control_expr(stream, content.to_owned(), pos.0),
            _ => Ok(Box::new(VariableExpression::new(
                identifier_token.pos,
                identifier_token.content,
            ))),
        }
    }

    fn parse_open_bracket_token_expr(stream: &mut TokenStream) -> ExprResult {
        let open_pos = match Parser::expect_token(TokenType::OpenBracket, stream.next()) {
            Ok(token) => token.pos.0,
            Err(err) => return Err(err),
        };

        let mut is_first_iter = true;
        let mut expressions = Vec::new();
        let close_pos = loop {
            match stream.peek() {
                // if the next token is an end bracket, the sub-expression has ended
                Some(Token {
                    token_type: TokenType::CloseBracket,
                    pos,
                    ..
                }) => break pos.1,
                None => return Err(CompileError::UnexpectedEnd),
                _ => (),
            };

            // this iteration should start with a comma if it's not the first
            if !is_first_iter {
                if let Err(err) = Parser::expect_token(TokenType::Comma, stream.next()) {
                    return Err(err);
                }
            } else {
                is_first_iter = false;
            }

            expressions.push(match Parser::parse_expression(stream, PRECEDENCE_ALL) {
                Ok(expr) => expr,
                Err(err) => return Err(err),
            });
        };

        // consume the last token (an end bracket)
        stream.next();

        // if only one expression was entered return that expression, otherwise provide the tuple
        if expressions.len() == 1 {
            Ok(expressions.remove(0))
        } else {
            Ok(Box::new(TupleExpression::new(
                SourceRange(open_pos, close_pos),
                expressions,
            )))
        }
    }

    fn parse_control_expr(
        stream: &mut TokenStream,
        name: String,
        start_pos: SourcePos,
    ) -> ExprResult {
        // ensure the expression starts with a colon
        if let Err(err) = Parser::expect_token(TokenType::Colon, stream.next()) {
            return Err(err);
        }

        let type_token = match Parser::expect_token(TokenType::Identifier, stream.next()) {
            Ok(token) => token,
            Err(err) => return Err(err),
        };

        let control_type = match type_token.content.as_ref() {
            "num" => ControlType::Audio,
            "graph" => ControlType::Graph,
            "midi" => ControlType::Midi,
            "roll" => ControlType::Roll,
            "scope" => ControlType::Scope,
            "num[]" => ControlType::NumExtract,
            "midi[]" => ControlType::MidiExtract,
            _ => {
                return Err(CompileError::unknown_control(
                    type_token.content,
                    type_token.pos,
                ))
            }
        };

        // parse the property name
        let (prop_name, end_pos) = match stream.peek() {
            Some(Token {
                token_type: TokenType::Dot,
                ..
            }) => {
                // consume the dot token and get the next one
                stream.next();

                match Parser::expect_token(TokenType::Identifier, stream.next()) {
                    Ok(Token { content, pos, .. }) => (content, pos.1),
                    Err(err) => return Err(err),
                }
            }
            _ => ("value".to_owned(), type_token.pos.1),
        };

        Ok(Box::new(ControlExpression::new(
            SourceRange(start_pos, end_pos),
            name,
            control_type,
            prop_name,
        )))
    }

    fn parse_call_expr(stream: &mut TokenStream, name: String, start_pos: SourcePos) -> ExprResult {
        if let Err(err) = Parser::expect_token(TokenType::OpenBracket, stream.next()) {
            return Err(err);
        }

        let args = match stream.peek() {
            Some(Token {
                token_type: TokenType::CloseBracket,
                ..
            })
            | None => Vec::new(),
            _ => match Parser::parse_arg_list(stream) {
                Ok(args) => args,
                Err(err) => return Err(err),
            },
        };

        let end_pos = match Parser::expect_token(TokenType::CloseBracket, stream.next()) {
            Ok(token) => token.pos.1,
            Err(err) => return Err(err),
        };

        Ok(Box::new(CallExpression::new(
            SourceRange(start_pos, end_pos),
            name,
            args,
        )))
    }

    fn parse_arg_list(stream: &mut TokenStream) -> CompileResult<Vec<Box<Expression>>> {
        let mut result = Vec::new();
        let mut is_first_iter = true;
        loop {
            // if this is not the first iteration, only continue if there isn't a comma
            if !is_first_iter {
                match stream.peek() {
                    Some(Token {
                        token_type: TokenType::Comma,
                        ..
                    }) => (),
                    _ => break,
                }

                // consume the comma we just read
                stream.next();
            } else {
                is_first_iter = false;
            }

            match Parser::parse_expression(stream, PRECEDENCE_ALL) {
                Ok(expr) => result.push(expr),
                Err(err) => return Err(err),
            }
        }

        Ok(result)
    }

    fn parse_cast_expr(stream: &mut TokenStream, lhs: Box<Expression>) -> ExprResult {
        if let Err(err) = Parser::expect_token(TokenType::Cast, stream.next()) {
            return Err(err);
        }

        let form = match Parser::parse_form(stream) {
            Ok(form) => form,
            Err(err) => return Err(err),
        };
        Ok(Box::new(CastExpression::new(
            SourceRange(lhs.pos().0, form.pos().1),
            form,
            lhs,
            false,
        )))
    }

    fn parse_postfix_expr(stream: &mut TokenStream, lhs: &Expression) -> ExprResult {
        let (operation, end_pos) = match stream.next() {
            Some(Token {
                token_type: TokenType::Increment,
                pos,
                ..
            }) => (PostfixOperation::Increment, pos.1),
            Some(Token {
                token_type: TokenType::Decrement,
                pos,
                ..
            }) => (PostfixOperation::Decrement, pos.1),
            Some(token) => return Err(CompileError::unexpected_token(token)),
            None => return Err(CompileError::UnexpectedEnd),
        };

        match Parser::get_lvalue(lhs) {
            Ok(lvalue) => Ok(Box::new(PostfixExpression::new(
                SourceRange(lhs.pos().0, end_pos),
                lvalue,
                operation,
            ))),
            Err(err) => Err(err),
        }
    }

    fn parse_math_expr(stream: &mut TokenStream, lhs: Box<Expression>) -> ExprResult {
        let (operator, token_type) = match stream.next() {
            Some(token) => {
                let op = match token.token_type {
                    TokenType::BitwiseAnd => OperatorType::BitwiseAnd,
                    TokenType::BitwiseOr => OperatorType::BitwiseOr,
                    TokenType::BitwiseXor => OperatorType::BitwiseXor,
                    TokenType::LogicalAnd => OperatorType::LogicalAnd,
                    TokenType::LogicalOr => OperatorType::LogicalOr,
                    TokenType::EqualTo => OperatorType::LogicalEqual,
                    TokenType::NotEqualTo => OperatorType::LogicalNotEqual,
                    TokenType::Lt => OperatorType::LogicalLt,
                    TokenType::Gt => OperatorType::LogicalGt,
                    TokenType::Lte => OperatorType::LogicalLte,
                    TokenType::Gte => OperatorType::LogicalGte,
                    TokenType::Plus => OperatorType::Add,
                    TokenType::Minus => OperatorType::Subtract,
                    TokenType::Divide => OperatorType::Divide,
                    TokenType::Modulo => OperatorType::Modulo,
                    TokenType::Power => OperatorType::Power,
                    _ => return Err(CompileError::unexpected_token(token)),
                };
                (op, token.token_type)
            }
            None => return Err(CompileError::UnexpectedEnd),
        };

        let rhs =
            match Parser::parse_expression(stream, Parser::get_operator_precedence(token_type)) {
                Ok(expr) => expr,
                Err(err) => return Err(err),
            };
        Ok(Box::new(MathExpression::new(
            SourceRange(lhs.pos().0, rhs.pos().1),
            lhs,
            rhs,
            operator,
        )))
    }

    fn parse_assign_expr(stream: &mut TokenStream, lhs: &Expression) -> ExprResult {
        let (operator, token_type) = match stream.next() {
            Some(token) => {
                let op = match token.token_type {
                    TokenType::Assign => OperatorType::Identity,
                    TokenType::PlusAssign => OperatorType::Add,
                    TokenType::MinusAssign => OperatorType::Subtract,
                    TokenType::TimesAssign => OperatorType::Multiply,
                    TokenType::DivideAssign => OperatorType::Divide,
                    TokenType::ModuloAssign => OperatorType::Modulo,
                    TokenType::PowerAssign => OperatorType::Power,
                    _ => return Err(CompileError::unexpected_token(token)),
                };
                (op, token.token_type)
            }
            None => return Err(CompileError::UnexpectedEnd),
        };

        let lvalue = match Parser::get_lvalue(lhs) {
            Ok(expr) => expr,
            Err(err) => return Err(err),
        };
        let rhs =
            match Parser::parse_expression(stream, Parser::get_operator_precedence(token_type)) {
                Ok(expr) => expr,
                Err(err) => return Err(err),
            };
        Ok(Box::new(AssignExpression::new(
            SourceRange(lhs.pos().0, rhs.pos().1),
            lvalue,
            rhs,
            operator,
        )))
    }

    fn get_lvalue(expr: &Expression) -> CompileResult<LValueExpression> {
        match expr.assignables() {
            Ok(assignables) => Ok(LValueExpression::new(*expr.pos(), assignables)),
            Err(range) => Err(CompileError::required_assignable(range)),
        }
    }

    fn expect_token(expected_type: TokenType, token_stream: Option<Token>) -> CompileResult<Token> {
        match token_stream {
            Some(token) => {
                if token.token_type == expected_type {
                    Ok(token)
                } else {
                    Err(CompileError::mismatched_token(expected_type, token))
                }
            }
            None => Err(CompileError::UnexpectedEnd),
        }
    }

    fn get_operator_precedence(token_type: TokenType) -> i32 {
        match token_type {
            TokenType::Cast => PRECEDENCE_CASTING,
            TokenType::Increment | TokenType::Decrement => PRECEDENCE_UNARY,
            TokenType::BitwiseAnd | TokenType::BitwiseOr | TokenType::BitwiseXor => {
                PRECEDENCE_BITWISE
            }
            TokenType::Plus => PRECEDENCE_ADD,
            TokenType::Minus => PRECEDENCE_SUBTRACT,
            TokenType::Times => PRECEDENCE_MULTIPLY,
            TokenType::Divide => PRECEDENCE_DIVIDE,
            TokenType::Modulo => PRECEDENCE_MODULO,
            TokenType::Power => PRECEDENCE_POWER,
            TokenType::EqualTo
            | TokenType::NotEqualTo
            | TokenType::Lt
            | TokenType::Gt
            | TokenType::Lte
            | TokenType::Gte => PRECEDENCE_EQUALITY,
            TokenType::LogicalAnd | TokenType::LogicalOr => PRECEDENCE_LOGICAL,
            TokenType::Assign
            | TokenType::PlusAssign
            | TokenType::MinusAssign
            | TokenType::TimesAssign
            | TokenType::DivideAssign
            | TokenType::ModuloAssign
            | TokenType::PowerAssign => PRECEDENCE_ASSIGNMENT,
            _ => PRECEDENCE_ALL,
        }
    }
}
