use crate::core::{expression::runa_expression, *};

pub fn runa_expression_tables(runa: &mut Runa, _: &String) -> (bool, RunaValue) {
    let mut table = Vec::new();
    let mut i = 1;

    loop {
        let key_token = lexer::next(runa);
        if key_token.is_eof() { return ( false, RunaValue::Nil ); }

        if let Some("}") = key_token.as_str_ref() { return ( true, RunaValue::Table(internal::gen_id(), table) ); }

        let after = lexer::next(runa);

        if let Some("=") = after.as_str_ref() {
            let key = match key_token.as_str() {
                Some(k) => k.clone(),
                None => return (false, RunaValue::Nil),
            };

            let value_token = lexer::next(runa);
            let (success, value) = runa_expression(runa, value_token.as_str().unwrap());
            if !success { return ( false, RunaValue::Nil ); }

            let addr = internal::gen_id();
            runa_push_local(runa, Local::Variable(Variable {
                name: format!("runa:{}", addr.clone()),
                value
            }));

            let ptr = RunaValue::Pointer(addr);
            table.push((key, Rc::new(RefCell::new(ptr))));

            let sep = lexer::next(runa);
            match sep.as_str_ref() {
                Some(",") => continue,
                Some("}") => return ( true, RunaValue::Table(internal::gen_id(), table) ),
                _ => {
                    lexer::back(runa, sep);
                    return ( true, RunaValue::Table(internal::gen_id(), table) );
                }
            }
        }

        lexer::back(runa, after);

        let (success, value) = runa_expression(runa, key_token.as_str().unwrap());
        if !success { return ( false, RunaValue::Nil ); }

        let addr = internal::gen_id();
        runa_push_local(runa, Local::Variable(Variable {
            name: format!("runa:{}", addr.clone()),
            value
        }));

        let ptr = RunaValue::Pointer(addr);
        table.push((i.to_string(), Rc::new(RefCell::new(ptr))));
        i += 1;

        let sep = lexer::next(runa);
        match sep.as_str_ref() {
            Some(",") => continue,
            Some("}") => return ( true, RunaValue::Table(internal::gen_id(), table) ),
            _ => {
                lexer::back(runa, sep);
                return ( true, RunaValue::Table(internal::gen_id(), table) );
            }
        }
    }
}
