use crate::core::{lexer::{Token}, *};

pub fn isstring(token: &String) -> bool {
    (token.starts_with('"') && token.ends_with('"')) || (token.starts_with('\'') && token.ends_with('\''))
}

pub fn runa_expression(runa: &mut Runa, token: &String) -> RunaValue {
    if isstring(token) {
        let next = lexer::next(runa);
        if next.is_eof() {
            return RunaValue::String(token[1..token.len()-1].to_string());
        }

        let mut buffer = token[1..token.len()-1].to_string();
        let mut operator: Token = next.clone();
        loop {
            if operator.as_str().unwrap() != ".." {
                lexer::back(runa, operator);
                return RunaValue::String(buffer);
            }

            let rhs = lexer::next(runa);
            if rhs.is_eof() {
                runa_spawn_fatal_error(format!("Unexpected end of input after '..' in string literal: {}", buffer));
            }

            let text = rhs.as_str().unwrap();
            buffer.push_str(&text[1..text.len()-1]);

            operator = lexer::next(runa).clone();
        }

    }

    if isidentifier(token) {

    }

    unreachable!();
}
