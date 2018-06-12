use parser::{Token, TokenType, TokenStream, get_token_stream};
use ast::{Block, Expression};
use CompileError;
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

pub struct Parser {
    content: String
}

impl Parser {
    pub fn new(content: String) -> Parser {
        Parser {
            content
        }
    }

    pub fn parse(&self) -> Result<Block, CompileError> {
        let mut stream = get_token_stream(&self.content);
        let mut expressions = Vec::new();

        // todo: this is probably really bad and can be made much nicer
        loop {
            let peek_token = stream.peek().cloned();
            match peek_token {
                Some(ref token) => {
                    match token.get_token_type() {
                        TokenType::EndOfLine => {},
                        _ => {
                            match Parser::parse_expression(&mut stream, PRECEDENCE_ALL) {
                                Ok(expr) => expressions.push(expr),
                                Err(err) => return Err(err)
                            }

                            // ensure next token is a newline or end of stream
                            let next_token = stream.next();
                            match next_token {
                                Some(ref nt) => {
                                    match nt.get_token_type() {
                                        TokenType::EndOfLine => {},
                                        _ => return Err(CompileError::unexpected_token(TokenType::EndOfLine, nt.clone()))
                                    }
                                },
                                None => break
                            }
                        }
                    }
                },
                None => break
            }
        }

        Ok(Block::new(expressions))
    }

    fn parse_expression(stream: &mut TokenStream, precedence: i32) -> Result<Box<Expression>, CompileError> {
        // todo: there's probably a nice iterator-based solution for this
        let prefix = match Parser::parse_prefix(stream, precedence) {
            Ok(prefix) => prefix,
            Err(err) => return Err(err)
        };

        repeat(()).scan(prefix, |state, _| {
            let new_state = Parser::parse_postfix(stream, *state, precedence);
            
        });

        /*loop {
            result = match Parser::parse_postfix(stream, result, precedence) {
                Ok(Some(postfix)) => postfix,
                Ok(None) => break,
                Err(err) => return Err(err)
            };
        }*/

        Ok(result)
    }

    fn parse_prefix(stream: &mut TokenStream, precedence: i32) -> Result<Box<Expression>, CompileError> {
        unimplemented!();
    }

    fn parse_postfix(stream: &mut TokenStream, prefix: Box<Expression>, precedence: i32) -> Result<Option<Box<Expression>>, CompileError> {
        unimplemented!();
    }

    //fn parse_prefix(&self, stream: )
}
