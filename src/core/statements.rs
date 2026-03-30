use crate::core::{expression::runa_expression, lexer::Token, *};

pub fn statement(runa: &mut Runa, token: &String) -> bool {
    match token.as_str() {
        "if" => if_statement(runa),
        _ => false,
    }
}

fn if_statement(runa: &mut Runa) -> bool {
    if runa.modularization.len() == 0 {
        runa.modularization.push(0);
    }
    const RUNA_NOT: i32 = 0;
    const RUNA_AND: i32 = 1;
    const RUNA_OR: i32 = 2;
    let mut resolve = false;
    loop {
        let err_at = |str: &str| { runa_spawn_fatal_error(format!("An error was ocurred at in if statement in {str} value")); };

        let first_tk = lexer::next(runa);
        let Token::Value(mut first) = first_tk else { return false; };
        if first == "not" {
            *runa.modularization.last_mut().unwrap() |= 1 << RUNA_NOT;
            first = lexer::next(runa)
                        .as_str()
                        .unwrap()
                        .into();
        }

        let (success_0, lhs) = runa_expression(runa, &first);
        if !success_0 { err_at("LHS") }

        let operator = lexer::next(runa);
        if operator.is_eof() { err_at("Comparison operator") }

        let second_tk = lexer::next(runa);
        let Token::Value(second) = second_tk else { return false; };
        let (success_1, rhs) = runa_expression(runa, &second);
        if !success_1 { err_at("RHS") }

        let op = operator.as_str_ref().unwrap();
        let result = match op {
            "==" => lhs == rhs,
            "~=" => lhs != rhs,
            ">" | "<" | "<=" | ">=" => {
                match (&lhs, &rhs) {
                    /* Integer comparison */
                    (RunaValue::Integer(l), RunaValue::Integer(r)) => match op {
                        ">" => l > r,
                        "<" => l < r,
                        ">=" => l >= r,
                        "<=" => l <= r,
                        _ => unreachable!()
                    },

                    /* String length comparison */
                    (RunaValue::String(l), RunaValue::String(r)) => match op {
                        ">" => l.len() > r.len(),
                        "<" => l.len() < r.len(),
                        ">=" => l.len() >= r.len(),
                        "<=" => l.len() <= r.len(),
                        _ => unreachable!()
                    }
                    _ => {
                        runa_spawn_fatal_error(format!(
                            "You can't compare a {} with a {}",
                            lhs.kind_to_string(), rhs.kind_to_string()
                        ));
                        unreachable!();
                    },
                }
            },
            _ => false,
        };

        let not_flag = runa.modularization.last().unwrap() & (1 << RUNA_NOT) != 0;
        if runa.modularization.last().unwrap() & 1 << RUNA_AND != 0 {
            *runa.modularization.last_mut().unwrap() &= !(1 << RUNA_AND);
            resolve = resolve && if not_flag { !result } else { result };
        }
        else if runa.modularization.last().unwrap() & 1 << RUNA_OR != 0 {
            *runa.modularization.last_mut().unwrap() &= !(1 << RUNA_OR);
            resolve = resolve || if not_flag { !result } else { result };
        }
        else { resolve = if not_flag { !result } else { result }; }

        let third_tk = lexer::next(runa);
        let Token::Value(third) = third_tk else { return false; };
        match third.as_str() {
            "then" => break,
            "and" => *runa.modularization.last_mut().unwrap() |= 1 << RUNA_AND,
            "or" => *runa.modularization.last_mut().unwrap() |= 1 << RUNA_OR,
            _ => return false,
        }
    }


    let mut i = 0;
    if !resolve {loop {
        let tk = lexer::next(runa);
        let Token::Value(value) = tk else {
            runa_spawn_fatal_error(format!("Expected 'end', got EOF in if statement"));
            unreachable!()
        };

        match value.as_str() {
            "end" if i == 0 => break,
            "if" | "do" | "function" => i += 1,
            "end" => i -= 1,
            _ => {},
        }
    }}

    return true;
}
