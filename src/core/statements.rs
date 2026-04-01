use crate::core::{expression::runa_expression, lexer::Token, *};

pub fn statement(runa: &mut Runa, token: &String) -> bool {
    match token.as_str() {
        "if" => if_statement(runa),
        "else" => else_statement(runa),
        "elseif" => elseif_statement(runa),
        "function" => function_statement(runa),
        "return" => return_statement(runa),
        "for" => for_statement(runa),
        "while" => while_statement(runa),
        _ => false,
    }
}

fn eval_condition(runa: &mut Runa, stop_word: &str) -> bool {
    if runa.modularization.len() == 0 {
        runa.modularization.push(0);
    }

    const RUNA_NOT: i32 = 0;
    const RUNA_AND: i32 = 1;
    const RUNA_OR: i32 = 2;

    let mut resolve = false;

    loop {
        let first_tk = lexer::next(runa);
        let Token::Value(mut first) = first_tk else { return false };

        if first == "not" {
            *runa.modularization.last_mut().unwrap() |= 1 << RUNA_NOT;
            first = lexer::next(runa).as_str().unwrap().into();
        }

        let (_, lhs) = runa_expression(runa, &first);

        let operator = lexer::next(runa);
        let second_tk = lexer::next(runa);

        let Token::Value(second) = second_tk else { return false };
        let (_, rhs) = runa_expression(runa, &second);

        let op = operator.as_str_ref().unwrap();

        let result = match op {
            "==" => lhs == rhs,
            "~=" => lhs != rhs,
            ">" | "<" | "<=" | ">=" => {
                match (&lhs, &rhs) {
                    (RunaValue::Integer(l), RunaValue::Integer(r)) => match op {
                        ">" => l > r,
                        "<" => l < r,
                        ">=" => l >= r,
                        "<=" => l <= r,
                        _ => unreachable!()
                    },
                    _ => false
                }
            },
            _ => false,
        };

        let not_flag = runa.modularization.last().unwrap() & (1 << RUNA_NOT) != 0;

        if runa.modularization.last().unwrap() & (1 << RUNA_AND) != 0 {
            *runa.modularization.last_mut().unwrap() &= !(1 << RUNA_AND);
            resolve = resolve && if not_flag { !result } else { result };
        } else if runa.modularization.last().unwrap() & (1 << RUNA_OR) != 0 {
            *runa.modularization.last_mut().unwrap() &= !(1 << RUNA_OR);
            resolve = resolve || if not_flag { !result } else { result };
        } else {
            resolve = if not_flag { !result } else { result };
        }

        let third_tk = lexer::next(runa);
        let Token::Value(third) = third_tk else { return false };

        match third.as_str() {
            x if x == stop_word => break,
            "and" => *runa.modularization.last_mut().unwrap() |= 1 << RUNA_AND,
            "or" => *runa.modularization.last_mut().unwrap() |= 1 << RUNA_OR,
            _ => return false,
        }
    }

    resolve
}

fn if_statement(runa: &mut Runa) -> bool {
    if runa.modularization.len() == 0 { runa.modularization.push(0); }
    let resolve = eval_condition(runa, "then");

    let mut i = 0;
    if !resolve {loop {
        let tk = lexer::next(runa);
        let Token::Value(value) = &tk else {
            runa_spawn_fatal_error(format!("Expected 'end', got EOF in if statement"));
            unreachable!()
        };

        match value.as_str() {
            "end" if i == 0 => break,
            "else" | "elseif" if i == 0 => {
                lexer::back(runa, tk);
                break;
            },
            "if" | "do" | "function" => i += 1,
            "end" => i -= 1,
            _ => {},
        }
    }}

    return true;
}

pub fn else_statement(runa: &mut Runa) -> bool {
    let last = runa.if_resolved.pop().unwrap();
    if last {
        let mut i = 0;
        loop {
            let tk = lexer::next(runa);
            let Token::Value(value) = tk else {
                runa_spawn_fatal_error(format!("Expected 'end', got EOF in else statement"));
                unreachable!()
            };

            match value.as_str() {
                "end" if i == 0 => break,
                "if" | "do" | "function" => i += 1,
                "end" => i -= 1,
                _ => {},
            }
        }
    }
    return true;
}

pub fn elseif_statement(runa: &mut Runa) -> bool {
    let last = runa.if_resolved.pop().unwrap();
    if last {
        let mut i = 0;
        loop {
            let tk = lexer::next(runa);
            let Token::Value(value) = tk else {
                runa_spawn_fatal_error(format!("Expected 'end', got EOF in elseif statement"));
                unreachable!()
            };

            match value.as_str() {
                "end" if i == 0 => break,
                "if" | "do" | "function" => i += 1,
                "end" => i -= 1,
                _ => {},
            }
        }
        return true;
    }
    return if_statement(runa);
}


pub fn function_statement(runa: &mut Runa) -> bool {
    let identifier_tk = lexer::next(runa);
    let Token::Value(identifier) = identifier_tk else {
        runa_spawn_fatal_error(format!("Expected identifier, got EOF"));
        unreachable!()
    };

    if !isidentifier(&identifier) {
        runa_spawn_fatal_error(format!("Expected identifier, got '{}'", identifier));
        return false;
    }

    let open_tk = lexer::next(runa);
    let Token::Value(open_value) = open_tk else {
        runa_spawn_fatal_error(format!("Expected '(', got EOF"));
        unreachable!()
    };

    if open_value != "(" { return false; }

    let mut argc = 0;
    let mut argv = Vec::new();
    loop {
        let tk = lexer::next(runa);
        let Token::Value(value) = tk else {
            runa_spawn_fatal_error(format!("Expected ')', got EOF in function statement"));
            unreachable!()
        };

        if value == ")" { break; }

        if !isidentifier(&value) {
            runa_spawn_fatal_error(format!("Expected identifier, got '{}' in function arguments", value));
            return false;
        }

        argv.push(value.clone());

        let comma = lexer::next(runa);
        let Token::Value(comma_value) = comma else {
            runa_spawn_fatal_error(format!("Expected ',', got EOF in function arguments"));
            unreachable!()
        };

        argc += 1;
        if comma_value == ")" { break; }
        if comma_value == "," { continue; }

        runa_spawn_fatal_error(format!("Expected ',' or ')', got '{}' in function arguments", comma_value));
    }

    let mut code = String::new();
    let mut i = 0;
    loop {
        let tk = lexer::next(runa);
        let Token::Value(value) = tk else {
            runa_spawn_fatal_error(format!("Expected 'end', got EOF in function statement"));
            unreachable!()
        };

        match value.as_str() {
            "end" if i == 0 => break,
            "if" | "do" | "function" => i += 1,
            "end" => i -= 1,
            _ => {},
        }

        code.push_str(&value);
        code.push('\n');
    }

    let compacted = lzma::compress(code.as_bytes()).unwrap();
    runa_push_local(runa, Local::Function(Function {
        argc: argc,
        argv: Some(argv),
        name: identifier.clone(),
        std: false,
        body: Some(compacted.clone()),
        ..Default::default()
    }));

    return true;
}

pub fn return_statement(runa: &mut Runa) -> bool {
    let tk = lexer::next(runa);
    let Token::Value(value) = tk else {
        return false;
    };

    let (success, value) = runa_expression(runa, &value);
    if !success {
        runa_spawn_fatal_error(format!("Fail to return expression"));
        return false;
    }

    runa.return_value = Some(value);
    runa.should_leave = true;
    return false;
}

pub fn for_statement(runa: &mut Runa) -> bool {
    let tk = lexer::next(runa);
    let Token::Value(var_name) = tk else { return false };

    if !isidentifier(&var_name) {
        runa_spawn_fatal_error(format!("Expected identifier in for"));
        return false;
    }

    let eq = lexer::next(runa);
    if eq.as_str().unwrap() != "="
    { runa_spawn_fatal_error(format!("Expected '=' in for")); }

    let start_tk = lexer::next(runa);
    let Token::Value(start_val) = start_tk else { return false };
    let (_, start) = runa_expression(runa, &start_val);

    let comma = lexer::next(runa);
    if comma.as_str().unwrap() != ","
    { runa_spawn_fatal_error(format!("Expected ',' in for")); }

    let end_tk = lexer::next(runa);
    let Token::Value(end_val) = end_tk else { return false };
    let (_, end) = runa_expression(runa, &end_val);

    let mut step = 1;

    let next = lexer::next(runa);
    let mut next_val = next.as_str().unwrap().to_string();

    if next_val == "," {
        let step_tk = lexer::next(runa);
        let Token::Value(step_val) = step_tk else { return false };
        let (_, step_expr) = runa_expression(runa, &step_val);

        if let RunaValue::Integer(s) = step_expr { step = s; }
        else
        { runa_spawn_fatal_error(format!("Step must be integer")); }

        next_val = lexer::next(runa).as_str().unwrap().to_string();
    }

    if next_val != "do"
    { runa_spawn_fatal_error(format!("Expected 'do' in for")); }

    let mut code = String::new();
    let mut i = 0;

    loop {
        let tk = lexer::next(runa);
        let Token::Value(value) = tk else {
            runa_spawn_fatal_error(format!("Expected 'end' in for statement"));
            unreachable!()
        };

        match value.as_str() {
            "end" if i == 0 => break,
            "if" | "do" | "function" | "for" => i += 1,
            "end" => i -= 1,
            _ => {}
        }

        code.push_str(&value);
        code.push('\n');
    }

    let (start, end) = match (start, end) {
        (RunaValue::Integer(s), RunaValue::Integer(e)) => (s, e),
        _ => {
            runa_spawn_fatal_error(format!("for only supports integers"));
            unreachable!()
        }
    };

    let compiled = lzma::compress(code.as_bytes()).unwrap();

    let mut i = start;
    while if step > 0 { i <= end } else { i >= end } {
        runa.stack.push(Vec::new());
        runa_push_local(runa, Local::Variable(Variable {
            name: var_name.clone(),
            value: RunaValue::Integer(i)
        }));

        lexer::branch(runa, compiled.clone());
        parse(runa);
        runa.stack.pop();

        i += step;
    }

    true
}


pub fn while_statement(runa: &mut Runa) -> bool {
    let mut condition_code = String::new();
    loop {
        let tk = lexer::next(runa);
        let Token::Value(value) = tk else {
            runa_spawn_fatal_error(format!("Expected 'do' in the condition of while statement"));
            unreachable!()
        };

        condition_code.push_str(&value);
        condition_code.push('\n');

        if value.as_str() == "do" { break; }
    }


    let mut code = String::new();
    let mut i = 0;

    loop {
        let tk = lexer::next(runa);
        let Token::Value(value) = tk else {
            runa_spawn_fatal_error(format!("Expected 'end' in while statement"));
            unreachable!()
        };

        match value.as_str() {
            "end" if i == 0 => break,
            "if" | "do" | "function" | "for" => i += 1,
            "end" => i -= 1,
            _ => {}
        }

        code.push_str(&value);
        code.push('\n');
    }

    let condition = lzma::compress(condition_code.as_bytes()).unwrap();
    let compiled = lzma::compress(code.as_bytes()).unwrap();

    loop {
        runa.stack.push(Vec::new());
        lexer::branch(runa, condition.clone());
        let resolve = eval_condition(runa, "do");
        if !resolve { break; }

        lexer::branch(runa, compiled.clone());
        parse(runa);
        runa.stack.pop();
    }

    true
}
