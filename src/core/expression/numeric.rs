use crate::core::{expression::*, parser::function};

fn precedence(op: &str) -> i32 {
    match op {
        "+" | "-" => 1,
        "*" | "/" | "%" | "//" => 2,
        _ => -1,
    }
}

fn is_operator(op: &str) -> bool {
    matches!(op, "+" | "-" | "*" | "/" | "%" | "//")
}

fn parse_primary(runa: &mut Runa, token: &str) -> RunaValue {
    if isstring(token) {
        runa_spawn_fatal_error("You cannot make this operation by a string".into());
    }

    if let Ok(v) = token.parse::<f64>() {
        return if v.fract() == 0.0 {
            RunaValue::Integer(v as isize)
        } else {
            RunaValue::Float(v)
        };
    }

    if isidentifier(token) {
        let next = lexer::next(runa);

        if next.as_str().unwrap() == "(" {
            _ = function(runa, &token.into());
            return RunaValue::Nil;
        }

        lexer::back(runa, next);

        return match runa_peek(runa, &token.into()) {
            Some(Local::Variable(var)) => var.value.clone(),
            Some(Local::Function(_)) => {
                runa_spawn_fatal_error(format!("Function `{}` used without call", token));
                unreachable!()
            }
            None => {
                runa_spawn_fatal_error(format!("Unknown identifier `{}`", token));
                unreachable!()
            }
        };
    }

    if token == "(" {
        let tk = lexer::next(runa);
        if tk.is_eof() {
            runa_spawn_fatal_error("Unexpected EOF after '('".into());
        }

        let (ok, val) = runa_expression(runa, tk.as_str().unwrap());
        if !ok {
            return RunaValue::Nil;
        }

        let close = lexer::next(runa);
        if close.as_str().unwrap() != ")" {
            runa_spawn_fatal_error("Expected ')'".into());
        }

        return val;
    }

    runa_spawn_fatal_error(format!("Unexpected token: {}", token));
    unreachable!()
}

fn apply_op(op: &str, left: f64, right: f64) -> f64 {
    match op {
        "+" => left + right,
        "-" => left - right,
        "*" => left * right,
        "/" => left / right,
        "%" => (left as i64 % right as i64) as f64,
        "//" => ((left / right) as i64) as f64,
        _ => unreachable!(),
    }
}

fn parse_expression_bp(runa: &mut Runa, mut left: RunaValue, min_bp: i32) -> RunaValue {
    loop {
        let op_token = lexer::next(runa);

        if op_token.is_eof() {
            return left;
        }

        let op = match op_token.as_str() {
            Some(s) => s,
            None => return left,
        };

        if op == ")" {
            lexer::back(runa, op_token);
            return left;
        }

        if !is_operator(op) {
            lexer::back(runa, op_token);
            return left;
        }

        let prec = precedence(op);

        if prec < min_bp {
            lexer::back(runa, op_token);
            return left;
        }

        let right_bp = prec + 1;

        let next = lexer::next(runa);
        if next.is_eof() {
            runa_spawn_fatal_error("Unexpected EOF".into());
        }

        let mut right = parse_primary(runa, next.as_str().unwrap());
        right = parse_expression_bp(runa, right, right_bp);

        let a = runa_value_to_string(&left).parse::<f64>().unwrap();
        let b = runa_value_to_string(&right).parse::<f64>().unwrap();

        let result = apply_op(op, a, b);

        left = if result.fract() == 0.0 {
            RunaValue::Integer(result as isize)
        } else {
            RunaValue::Float(result)
        };
    }
}

pub fn runa_expression_numeric(runa: &mut Runa, token: &String) -> (bool, RunaValue) {
    let left = parse_primary(runa, token);
    let result = parse_expression_bp(runa, left, 0);
    (true, result)
}
