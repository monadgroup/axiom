mod token;
mod token_stream;

pub use self::token::{Token, TokenType};
pub use self::token_stream::{get_token_stream, TokenStream};

use ast::*;
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
    Continue(Expression),
    End(Expression),
}

impl ParsedExpr {
    pub fn continue_expr(result: ExprResult) -> CompileResult<ParsedExpr> {
        match result {
            Ok(expr) => Ok(ParsedExpr::Continue(expr)),
            Err(err) => Err(err),
        }
    }
}

pub struct Parser {}

type ExprResult = CompileResult<Expression>;

impl Parser {
    pub fn parse(mut stream: &mut TokenStream) -> CompileResult<Block> {
        let mut expressions = Vec::new();

        loop {
            match stream.peek().cloned() {
                Some(Token {
                    token_type: TokenType::EndOfLine,
                    ..
                }) => {
                    stream.next();
                }
                Some(_) => {
                    expressions.push(Parser::parse_expression(&mut stream, PRECEDENCE_ALL)?);

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
        let mut result = Parser::parse_prefix(stream)?;
        loop {
            match Parser::parse_postfix(stream, result, precedence)? {
                ParsedExpr::Continue(expr) => result = expr,
                ParsedExpr::End(expr) => return Ok(expr),
            }
        }
    }

    fn parse_form(stream: &mut TokenStream) -> CompileResult<Form> {
        // ensure it starts with an open square bracket
        let start_pos = Parser::expect_token(TokenType::OpenSquare, stream.next())?
            .pos
            .0;
        let Token {
            content: form_name,
            pos: name_pos,
            ..
        } = Parser::expect_token(TokenType::Identifier, stream.next())?;

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

        let end_pos = Parser::expect_token(TokenType::CloseSquare, stream.next())?
            .pos
            .1;
        Ok(Form::new(SourceRange(start_pos, end_pos), form_type))
    }

    fn parse_prefix(stream: &mut TokenStream) -> ExprResult {
        let first_token = stream.peek().cloned();

        match first_token {
            Some(Token {
                token_type: TokenType::Colon,
                ref pos,
                ..
            }) => Parser::parse_control_expr(stream, "".to_string(), pos.0),
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
        prefix: Expression,
        precedence: i32,
    ) -> CompileResult<ParsedExpr> {
        let token_type = match stream.peek() {
            Some(token) => token.token_type,
            None => return Ok(ParsedExpr::End(prefix)),
        };

        // if this token has a lower precedence than provided, end this expression
        if precedence <= Parser::get_operator_precedence(token_type) {
            return Ok(ParsedExpr::End(prefix));
        }

        match token_type {
            TokenType::Cast => ParsedExpr::continue_expr(Parser::parse_cast_expr(stream, prefix)),
            TokenType::Increment | TokenType::Decrement => {
                ParsedExpr::continue_expr(Parser::parse_postfix_expr(stream, prefix))
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
                ParsedExpr::continue_expr(Parser::parse_assign_expr(stream, prefix))
            }
            _ => Ok(ParsedExpr::End(prefix)),
        }
    }

    fn parse_open_square_token_expr(stream: &mut TokenStream) -> ExprResult {
        let form = Parser::parse_form(stream)?;
        let expr = Parser::parse_expression(stream, PRECEDENCE_UNARY)?;

        let form_start = form.pos.0;
        let expr_end = expr.pos.1;

        Ok(Expression::new_cast(
            SourceRange(form_start, expr_end),
            form,
            Box::new(expr),
            true,
        ))
    }

    fn parse_note_token_expr(stream: &mut TokenStream) -> ExprResult {
        static NOTE_NAMES: [&str; 12] = [
            "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        ];
        lazy_static! {
            static ref NOTE_REGEX: Regex = Regex::new(r"([a-gA-G]#?)([0-9]+)").unwrap();
        }

        let note_token = Parser::expect_token(TokenType::Note, stream.next())?;
        let captures = NOTE_REGEX.captures(&note_token.content).unwrap();
        let note_name = captures[1].to_uppercase();
        let note_num = match NOTE_NAMES.iter().position(|&s| s == note_name) {
            Some(index) => index,
            None => return Err(CompileError::unknown_note(note_name, note_token.pos)),
        };
        let octave = captures[2].parse::<usize>().unwrap();
        let midi_num = note_num + octave * NOTE_NAMES.len();
        Ok(Expression::new_note(note_token.pos, midi_num as i32))
    }

    fn parse_number_token_expr(stream: &mut TokenStream) -> ExprResult {
        let num_token = Parser::expect_token(TokenType::Number, stream.next())?;
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

        Ok(Expression::new_number(
            SourceRange(num_token.pos.0, val_form.pos.1),
            num_val,
            val_form,
        ))
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

        let expr = Parser::parse_expression(stream, PRECEDENCE_UNARY)?;
        Ok(Expression::new_unary(
            SourceRange(pos.0, expr.pos.1),
            operator,
            Box::new(expr),
        ))
    }

    fn parse_identifier_token_expr(stream: &mut TokenStream) -> ExprResult {
        let identifier_token = Parser::expect_token(TokenType::Identifier, stream.next())?;

        match stream.peek().cloned() {
            Some(Token {
                token_type: TokenType::OpenBracket,
                pos,
                ..
            }) => Parser::parse_call_expr(stream, identifier_token.content, pos.0),
            Some(Token {
                token_type: TokenType::Colon,
                pos,
                ..
            }) => Parser::parse_control_expr(stream, identifier_token.content, pos.0),
            _ => Ok(Expression::new_variable(
                identifier_token.pos,
                identifier_token.content,
            )),
        }
    }

    fn parse_open_bracket_token_expr(stream: &mut TokenStream) -> ExprResult {
        let open_pos = Parser::expect_token(TokenType::OpenBracket, stream.next())?
            .pos
            .0;

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
                Parser::expect_token(TokenType::Comma, stream.next())?;
            } else {
                is_first_iter = false;
            }

            expressions.push(Parser::parse_expression(stream, PRECEDENCE_ALL)?);
        };

        // consume the last token (an end bracket)
        stream.next();

        // if only one expression was entered return that expression, otherwise provide the tuple
        if expressions.len() == 1 {
            Ok(expressions.remove(0))
        } else {
            Ok(Expression::new_tuple(
                SourceRange(open_pos, close_pos),
                expressions,
            ))
        }
    }

    fn parse_control_expr(
        stream: &mut TokenStream,
        name: String,
        start_pos: SourcePos,
    ) -> ExprResult {
        // ensure the expression starts with a colon
        Parser::expect_token(TokenType::Colon, stream.next())?;

        let type_token = Parser::expect_token(TokenType::Identifier, stream.next())?;
        let control_type = match type_token.content.as_ref() {
            "num" => ControlType::Audio,
            "graph" => ControlType::Graph,
            "midi" => ControlType::Midi,
            "roll" => ControlType::Roll,
            "scope" => ControlType::Scope,
            "num[]" => ControlType::AudioExtract,
            "midi[]" => ControlType::MidiExtract,
            _ => {
                return Err(CompileError::unknown_control(
                    type_token.content,
                    type_token.pos,
                ))
            }
        };

        // parse the property name
        let (prop_name, prop_pos) = match stream.peek() {
            Some(Token {
                token_type: TokenType::Dot,
                ..
            }) => {
                // consume the dot token and get the next one
                stream.next();

                let Token { content, pos, .. } =
                    Parser::expect_token(TokenType::Identifier, stream.next())?;
                (content, pos)
            }
            _ => ("value".to_string(), type_token.pos),
        };

        let control_field = match prop_name.as_ref() {
            "value" if control_type == ControlType::Audio => ControlField::Audio(AudioField::Value),
            "value" if control_type == ControlType::Graph => ControlField::Graph(GraphField::Value),
            "speed" if control_type == ControlType::Graph => ControlField::Graph(GraphField::Speed),
            "value" if control_type == ControlType::Midi => ControlField::Midi(MidiField::Value),
            "value" if control_type == ControlType::Roll => ControlField::Roll(RollField::Value),
            "speed" if control_type == ControlType::Roll => ControlField::Roll(RollField::Speed),
            "value" if control_type == ControlType::AudioExtract => {
                ControlField::AudioExtract(AudioExtractField::Value)
            }
            "value" if control_type == ControlType::MidiExtract => {
                ControlField::MidiExtract(MidiExtractField::Value)
            }
            _ => {
                return Err(CompileError::unknown_field(
                    control_type,
                    prop_name,
                    prop_pos,
                ))
            }
        };

        Ok(Expression::new_control(
            SourceRange(start_pos, prop_pos.1),
            name,
            control_field,
        ))
    }

    fn parse_call_expr(stream: &mut TokenStream, name: String, start_pos: SourcePos) -> ExprResult {
        Parser::expect_token(TokenType::OpenBracket, stream.next())?;

        let args = match stream.peek() {
            Some(Token {
                token_type: TokenType::CloseBracket,
                ..
            })
            | None => Vec::new(),
            _ => Parser::parse_arg_list(stream)?,
        };

        let end_pos = Parser::expect_token(TokenType::CloseBracket, stream.next())?
            .pos
            .1;

        Ok(Expression::new_call(
            SourceRange(start_pos, end_pos),
            name,
            args,
        ))
    }

    fn parse_arg_list(stream: &mut TokenStream) -> CompileResult<Vec<Expression>> {
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

            result.push(Parser::parse_expression(stream, PRECEDENCE_ALL)?);
        }

        Ok(result)
    }

    fn parse_cast_expr(stream: &mut TokenStream, lhs: Expression) -> ExprResult {
        Parser::expect_token(TokenType::Cast, stream.next())?;

        let form = Parser::parse_form(stream)?;
        Ok(Expression::new_cast(
            SourceRange(lhs.pos.0, form.pos.1),
            form,
            Box::new(lhs),
            false,
        ))
    }

    fn parse_postfix_expr(stream: &mut TokenStream, lhs: Expression) -> ExprResult {
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
            Ok(lvalue) => Ok(Expression::new_postfix(
                SourceRange(lvalue.pos.0, end_pos),
                lvalue,
                operation,
            )),
            Err(err) => Err(err),
        }
    }

    fn parse_math_expr(stream: &mut TokenStream, lhs: Expression) -> ExprResult {
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
                    TokenType::Times => OperatorType::Multiply,
                    TokenType::Divide => OperatorType::Divide,
                    TokenType::Modulo => OperatorType::Modulo,
                    TokenType::Power => OperatorType::Power,
                    _ => return Err(CompileError::unexpected_token(token)),
                };
                (op, token.token_type)
            }
            None => return Err(CompileError::UnexpectedEnd),
        };

        let rhs = Parser::parse_expression(stream, Parser::get_operator_precedence(token_type))?;
        Ok(Expression::new_math(
            SourceRange(lhs.pos.0, rhs.pos.1),
            Box::new(lhs),
            Box::new(rhs),
            operator,
        ))
    }

    fn parse_assign_expr(stream: &mut TokenStream, lhs: Expression) -> ExprResult {
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

        let lvalue = Parser::get_lvalue(lhs)?;
        let rhs = Parser::parse_expression(stream, Parser::get_operator_precedence(token_type))?;
        Ok(Expression::new_assign(
            SourceRange(lvalue.pos.0, rhs.pos.1),
            lvalue,
            Box::new(rhs),
            operator,
        ))
    }

    fn get_lvalue(expr: Expression) -> CompileResult<KnownExpression<LValueExpression>> {
        let expr_pos = expr.pos;
        match Parser::get_assignables(expr) {
            Ok(assignments) => Ok(KnownExpression::new(
                expr_pos,
                LValueExpression { assignments },
            )),
            Err(range) => Err(CompileError::required_assignable(range)),
        }
    }

    fn get_assignables(expr: Expression) -> Result<Vec<AssignableExpression>, SourceRange> {
        match expr.data {
            ExpressionData::Variable(var) => {
                Ok(vec![AssignableExpression::new_variable(expr.pos, var)])
            }
            ExpressionData::Control(control) => {
                Ok(vec![AssignableExpression::new_control(expr.pos, control)])
            }
            ExpressionData::Tuple(tuple) => {
                let mut assignables = Vec::new();
                for expr in tuple.expressions {
                    match Parser::get_assignables(expr) {
                        Ok(mut assigns) => assignables.append(&mut assigns),
                        Err(range) => return Err(range),
                    }
                }
                Ok(assignables)
            }
            _ => Err(expr.pos),
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
