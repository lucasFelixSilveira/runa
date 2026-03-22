use crate::core::{lexer::Token, expression::runa_expression, *};

pub fn identifier(runa: &mut Runa, token: &String) -> bool {
    if! isidentifier(token) { return false; }

    let next = lexer::next(runa);
    if next.is_eof() {
        lexer::back(runa, next);
        return true;
    }

    let ntk = next.as_str().unwrap();
    if ntk == "(" {
        let (is_std, callback, argc) = {
            let function = runa_peek_function(runa, token).unwrap();
            (function.std, function.callback, function.argc)
        };

        let mut args = Vec::new();

        while let Token::Value(arg) = lexer::next(runa) {
            if arg.as_str() == ")" { break; }
            args.push(runa_expression(runa, &arg));
        }

        if args.len() < argc {
            runa_spawn_fatal_error(format!("you tried to call `{token}` using {} arguments, but it expects {}", args.len(), argc));
        }

        if is_std {
            runa.args.push(args);
            if let Some(cb) = callback { cb(runa); }
            runa.args.pop();
        }
    }

    false
}

pub fn parse(runa: &mut Runa) {
    loop {
        let data = lexer::next(runa);
        if let lexer::Token::Value(token) = data {
            if identifier(runa, &token) { continue; }
            continue;
        }
        break;
    }
}
