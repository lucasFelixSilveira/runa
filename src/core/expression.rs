use crate::core::{parser::function, *};

mod string;
mod numeric;
mod table;

pub fn simulate_expression(runa: &mut Runa, val: &RunaValue) -> (bool, RunaValue) {
    match val {
        RunaValue::Pointer(_) => simulate_expression(runa, &runa_peek_value_to_pointer(runa, val.clone())),
        RunaValue::String(s) => string::runa_expression_string(runa, &format!("'{}'", s)),
        RunaValue::Integer(i) => numeric::runa_expression_numeric(runa, &i.to_string()),
        RunaValue::Float(f) => numeric::runa_expression_numeric(runa, &f.to_string()),
        RunaValue::Table(_, _) => {
            runa_spawn_fatal_error(format!("Internal runa error - Runa can't simulate a table"));
            unreachable!()
        }
        _ => (false, RunaValue::Nil),
    }
}

pub fn runa_expression(runa: &mut Runa, token: &String) -> (bool, RunaValue) {
    if token == "{"
    { return table::runa_expression_tables(runa, token); }

    if isstring(token)
    { return string::runa_expression_string(runa, token); }

    if isnumeric(token)
    { return numeric::runa_expression_numeric(runa, token); }

    if isidentifier(token) {
        let value = runa_peek(runa, token);
        if value.is_none()
        { return ( false, RunaValue::Nil ); }

        let local = value.unwrap();
        if let Local::Variable(var) = local {
            if let RunaValue::Table(_, _) = &var.value {
                let mut current = var.value.clone();
                let mut path = Vec::new();

                loop {
                    let operator = lexer::next(runa);

                    match operator.as_str_ref() {
                        Some("=") => {
                            let RunaValue::Pointer(addr) = &current else {
                                runa_spawn_fatal_error(format!("The field of this table isn't a value reference. This means: Internally Runa have did fail."));
                                unreachable!();
                            };

                            let expr = lexer::next(runa);
                            let (success, value) = runa_expression(runa, expr.as_str().unwrap_or_else(|| {
                                runa_spawn_fatal_error(format!("Has been expected a valid runa expression after `=` in the table assignment. Found `EOF`"));
                                unreachable!()
                            }));

                            if !success { return (false, RunaValue::Nil); }
                            runa_assign_local(runa, format!("runa:{addr}"), value.clone());
                            return (true, value)
                        }

                        Some(".") => {
                            let field_token = lexer::next(runa);
                            let field = match field_token.as_str() {
                                Some(f) => f,
                                None => return (false, RunaValue::Nil),
                            };

                            path.push(field.clone());

                            if let RunaValue::Table(_, table) = &current.clone() {
                                let val: Vec<_> = table.iter()
                                    .filter(|x| x.0 == *field)
                                    .collect();

                                if val.is_empty() { return (false, RunaValue::Nil); }

                                current = val[0].1.borrow().clone();
                            } else {
                                return (false, RunaValue::Nil);
                            }
                        }

                        Some("[") => {
                            let index_token = lexer::next(runa);
                            let (success, value) =
                                runa_expression(runa, index_token.as_str().unwrap());

                            if !success { return (false, RunaValue::Nil); }
                            if let RunaValue::Integer(index) = value {
                                if index <= 0 {
                                    runa_spawn_fatal_error(
                                        format!("You cannot access the index if it is <= 0 (1-based indexing) in the table:\n{}",
                                            runa_value_to_string(&var.value)
                                        )
                                    )
                                }
                                path.push(index.to_string());
                            }

                            let close = lexer::next(runa);
                            if close.as_str().unwrap() != "]" {
                                return (false, RunaValue::Nil);
                            }

                            if let RunaValue::Integer(index) = value {
                                if let RunaValue::Table(_, table) = &current.clone() {
                                    let key = index.to_string();

                                    let val: Vec<_> = table.iter()
                                        .filter(|x| x.0 == key)
                                        .collect();

                                    if val.is_empty() { return (false, RunaValue::Nil); }

                                    current = val[0].1.borrow().clone();
                                } else {
                                    return (false, RunaValue::Nil);
                                }
                            } else {
                                return (false, RunaValue::Nil);
                            }
                        }

                        _ => {
                            lexer::back(runa, operator);
                            break;
                        }
                    }
                }

                return simulate_expression(runa, &current);
            }

            let sign = lexer::next(runa);
            let lexer::Token::Value(op) = &sign else {
                return ( true, var.value.clone() );
            };

            return match op.as_str() {
                "=" => {
                    let expr = lexer::next(runa);
                    let (success, value) = runa_expression(runa, expr.as_str().unwrap_or_else(|| {
                        runa_spawn_fatal_error(format!("Has been expected a valid runa expression after `=` in the local assignment. Found `EOF`"));
                        unreachable!()
                    }));

                    if !success { return ( false, RunaValue::Nil ); }
                    runa_assign_local(runa, token.clone(), value.clone());
                    ( true, value )
                }
                _ => {
                    lexer::back(runa, sign);
                    simulate_expression(runa, &var.value)
                }
            };
        }

        if let Local::Function(_) = local {
            let paren = lexer::next(runa);
            if paren.as_str().unwrap() != "(" { return ( false, RunaValue::Nil ); }
            _ = function(runa, token);
            let data = runa.return_value.clone().unwrap_or_else(|| RunaValue::Nil);
            runa.return_value = None;
            return simulate_expression(runa, &data);
        }
    }

    ( false, RunaValue::Nil )
}
