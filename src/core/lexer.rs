use std::io::Read;

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
}

use crate::core::{Runa, isinteger};

static mut POSITION: i64 = 0;
static mut CODE_ID: i32 = 0;
static mut POS: [usize; 64] = [0; 64];

pub fn back(runa: &mut Runa, token: Token) {
    if let Token::Value(data) = token {
        runa.file.as_mut().unwrap().seek_relative(-(data.len() as i64)).ok();
    }
}

pub fn next(runa: &mut Runa) -> Token {
    let mut buffer = String::with_capacity(2048);

    unsafe {
        POSITION += 1;
    }

    if let Some(token) = runa.pushed.pop() {
        return token;
    }

    if !runa.code_stack.is_empty() {
        let code = runa.code_stack.last().unwrap().clone();

        let (locale, code_id) = unsafe { (POS[CODE_ID as usize], CODE_ID) };

        let chars: Vec<char> = code.chars().collect();
        let len = chars.len();

        let mut i = locale;

        while i < len {
            if i >= len.saturating_sub(2) {
                let _ = runa.code_stack.pop().unwrap();
                buffer.push(chars[i]);

                unsafe {
                    POS[code_id as usize] = 0;
                    CODE_ID -= 1;
                }

                runa.should_leave = true;
                return Token::Value(buffer);
            }

            if chars[i] == '\n' {
                break;
            }

            buffer.push(chars[i]);
            i += 1;
        }

        unsafe {
            POS[code_id as usize] = i + 1;
        }

        return Token::Value(buffer);
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
