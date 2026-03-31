use std::io::Read;
use crate::core::{Runa, isinteger, lzma};

#[derive(Clone, Debug, PartialEq)]
pub enum Token {
    Eof,
    Value(String),
}

impl Token {
    pub fn is_eof(&self) -> bool {
        matches!(self, Token::Eof)
    }

    pub fn is_value(&self) -> bool {
        matches!(self, Token::Value(_))
    }

    pub fn as_str(&self) -> Option<&String> {
        match self {
            Token::Value(s) => Some(&s),
            _ => None,
        }
    }

    pub fn as_str_ref(&self) -> Option<&str> {
        match self {
            Token::Value(s) => Some(s.as_str()),
            _ => None,
        }
    }
}

pub fn back(runa: &mut Runa, token: Token) {
    runa.pushed.push(token);
}

pub fn branch(runa: &mut Runa, body: Vec<u8>) {
    let decompressed = lzma::decompress(&body).unwrap();
    let data = String::from_utf8_lossy(&decompressed).into_owned();
    runa.bodies.push(data);
}

pub fn next(runa: &mut Runa) -> Token {
    let mut buffer = String::with_capacity(2048);

    if let Some(token) = runa.pushed.pop() {
        return token;
    }

    if runa.bodies.len() > 0 {
        let body = runa.bodies.first_mut().unwrap();
        let mem = (*body).clone();
        let (token, rest) = mem.split_once('\n').unwrap();
        if rest.is_empty() { runa.bodies.remove(0); }
        else { *body = rest.to_string(); }
        return Token::Value(token.to_string());
    }

    let mut byte = [0u8; 1];

    loop {
        if runa.file.as_mut().unwrap().read(&mut byte).unwrap() == 0 {
            return Token::Eof;
        }

        let c = byte[0] as char;

        if c == '"' || c == '\'' {
            let quote = c;
            buffer.push(quote);

            loop {
                if runa.file.as_mut().unwrap().read(&mut byte).unwrap() == 0 {
                    break;
                }

                let x = byte[0] as char;
                buffer.push(x);

                if x == quote {
                    break;
                }

                if x == '\\' {
                    if runa.file.as_mut().unwrap().read(&mut byte).unwrap() == 0 {
                        break;
                    }
                    buffer.push(byte[0] as char);
                }
            }

            return Token::Value(buffer);
        }

        if c.is_whitespace() {
            if !buffer.is_empty() {
                return Token::Value(buffer);
            }
            continue;
        }

        if !c.is_alphanumeric() && c != '_' {
            if !buffer.is_empty() && isinteger(&buffer) && c == '.' {
                buffer.push(c);
                continue;
            }

            if !buffer.is_empty() {
                runa.file.as_mut().unwrap().seek_relative(-1).ok();
                return Token::Value(buffer);
            }

            let mut buff = vec![c];

            if runa.file.as_mut().unwrap().read(&mut byte).unwrap() > 0 {
                let x = byte[0] as char;

                let is_double = (c == '=' && x == '=')
                    || (c == '!' && x == '=')
                    || (c == '<' && x == '=')
                    || (c == '>' && x == '=')
                    || (c == '-' && x == '-')
                    || (c == '.' && x == '.')
                    || (c == '&' && x == '&')
                    || (c == '/' && x == '/')
                    || (c == '|' && x == '|')
                    || (c == '~' && x == '=');

                if is_double {
                    buff.push(x);
                } else {
                    runa.file.as_mut().unwrap().seek_relative(-1).ok();
                }

                if c == '-' && x == '-' {
                    while runa.file.as_mut().unwrap().read(&mut byte).unwrap() > 0 {
                        if byte[0] as char == '\n' {
                            break;
                        }
                    }
                    continue;
                }
            }

            return Token::Value(buff.into_iter().collect());
        }

        buffer.push(c);

        if buffer.len() >= 2047 {
            break;
        }
    }

    if !buffer.is_empty() {
        return Token::Value(buffer);
    }

    Token::Value(String::new())
}
