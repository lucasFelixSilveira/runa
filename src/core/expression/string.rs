use crate::core::{expression::runa_expression, *};

pub fn runa_expression_string(runa: &mut Runa, token: &String) -> (bool, RunaValue) {
    let next = lexer::next(runa);
    if next.is_eof() {
        return ( true, RunaValue::String(token[1..token.len()-1].to_string()) );
    }

    let mut buffer = token[1..token.len()-1].to_string();
    let mut operator = next.clone();
    loop {
        if  operator.is_eof() || operator.as_str().unwrap() != ".." {
            lexer::back(runa, operator);
            return ( true, RunaValue::String(buffer) );
        }

        let rhs = lexer::next(runa);
        if rhs.is_eof()
        { runa_spawn_fatal_error(format!("Unexpected end of input after '..' in string literal: {}", buffer)); }

        let text = rhs.as_str().unwrap();
        let (_, data) = runa_expression(runa, &text.into());
        buffer.push_str(runa_value_to_string(&data).as_str());

        operator = lexer::next(runa).clone();
    }
}
