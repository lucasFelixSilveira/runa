use crate::core::{lexer::Token, expression::runa_expression, *};

pub fn function(runa: &mut Runa, token: &String) -> bool {
    let (is_std, callback, argc) = {
        let function = runa_peek_function(runa, token).unwrap();
        (function.std, function.callback, function.argc)
    };

    let mut args = Vec::new();

    loop {
        let tok = lexer::next(runa);
        if tok.is_eof()
        { runa_spawn_fatal_error("unexpected EOF inside function call".to_string()); }
        if let Token::Value(s) = &tok {
            if s == ")" { break; }
        }

        let (success, value) = runa_expression(runa, tok.as_str().unwrap());
        if !success
        { runa_spawn_fatal_error(format!("invalid expression in argument: {}", tok.as_str().unwrap())); }
        args.push(value);
    }

    if args.len() < argc {
        runa_spawn_fatal_error(format!(
            "you tried to call `{token}` using {} arguments, but it expects {}",
            args.len(), argc
        ));
    }

    if is_std {
        runa.args.push(args);
        if let Some(cb) = callback { cb(runa); }
        runa.args.pop();
    }

    return true;
}

pub fn identifier(runa: &mut Runa, token: &String) -> bool {
    if !isidentifier(token) {
        return false;
    }

    let next = lexer::next(runa);
    if next.is_eof() {
        lexer::back(runa, next);
        return true;
    }

    if let Token::Value(ntk) = &next {
        if ntk == "(" {
            return function(runa, token);
        }
    }

    lexer::back(runa, next);
    return true;
}

fn local(runa: &mut Runa, token: &String) -> bool {
    if token != "local" { return false; }

    let mut identifiers = Vec::new();
    loop {
        let tok = lexer::next(runa);
        if tok.is_eof() { break; }
        if let Token::Value(id) = tok {
            if !isidentifier(&id)
            { runa_spawn_fatal_error(format!("{id} isn't an identifier")); }
            identifiers.push(id);
        } else {
            lexer::back(runa, tok);
            break;
        }

        let sep = lexer::next(runa);
        if sep.is_eof() { break; }
        if let Token::Value(s) = &sep {
            if s == "," { continue; }
            if s == "=" {
                lexer::back(runa, sep);
                break;
            }
        }
        lexer::back(runa, sep);
        break;
    }

    for id in &identifiers {
        runa_push_local(runa, Local::Variable(Variable {
            name: id.clone(),
            value: RunaValue::Nil,
        }));
    }

    let next = lexer::next(runa);
    if next.is_eof() { return true; }

    if let Token::Value(s) = &next {
        if s != "=" {
            lexer::back(runa, next);
            return true;
        }
    } else {
        lexer::back(runa, next);
        return true;
    }

    let mut counter = 0;
    loop {
        let tok = lexer::next(runa);
        if tok.is_eof() { break; }

        let (success, value) = runa_expression(runa, tok.as_str().unwrap());
        if !success
        { runa_spawn_fatal_error(format!("expected expression after =, got {}", tok.as_str().unwrap())); }

        counter += 1;
        if counter <= identifiers.len()
        { runa_assign_local(runa, identifiers[counter - 1].clone(), value); }

        let sep = lexer::next(runa);
        if sep.is_eof() { break; }
        if let Token::Value(s) = &sep {
            if s != "," {
                lexer::back(runa, sep);
                break;
            }
        } else {
            lexer::back(runa, sep);
            break;
        }
    }

    if counter > identifiers.len() {
        runa_spawn_fatal_error(format!(
            "too many values in assignment: expected ≤ {}, got {}",
            identifiers.len(), counter
        ));
    }

    true
}

pub fn parse(runa: &mut Runa) {
    loop {
        let data = lexer::next(runa);
        if data.is_eof() { break; }

        if let Token::Value(token) = &data {
            if statements::statement(runa, token) { continue; }
            if local(runa, token) { continue; }
            if identifier(runa, token) { continue; }
        }

        runa_spawn_fatal_error(format!("unexpected token kind: {:?}", data));
    }
}
