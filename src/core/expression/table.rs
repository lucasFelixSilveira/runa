use crate::core::{expression::runa_expression, *};

pub fn runa_expression_tables(runa: &mut Runa, _: &String) -> (bool, RunaValue) {
    let mut table = Vec::new();
    let mut i = 1;

    loop {
        let key_token = lexer::next(runa);
        if key_token.is_eof() { return (false, RunaValue::Nil); }
        if let Some("}") = key_token.as_str_ref() { return (true, RunaValue::Table(table)); }

        let after = lexer::next(runa);
        if let Some("=") = after.as_str_ref() {
            let key = match key_token.as_str() {
                Some(k) => k.clone(),
                None => return (false, RunaValue::Nil),
            };

            let value_token = lexer::next(runa);
            let (success, value) = runa_expression(runa, value_token.as_str().unwrap());
            if !success { return ( false, RunaValue::Nil ); }

            table.push((key, Rc::new(RefCell::new(value))));

            let sep = lexer::next(runa);
            if let Some(",") = sep.as_str_ref() { continue; }
            if let Some("}") = sep.as_str_ref() { return (true, RunaValue::Table(table)); }
            return (false, RunaValue::Nil);
        }
        else {
            lexer::back(runa, after);

            let (success, value) = runa_expression(runa, key_token.as_str().unwrap());
            if !success { return (false, RunaValue::Nil); }

            table.push((i.to_string(), Rc::new(RefCell::new(value))));
            i += 1;

            let sep = lexer::next(runa);
            if let Some(",") = sep.as_str_ref() { continue; }
            if let Some("}") = sep.as_str_ref() { return (true, RunaValue::Table(table)); }
            return (false, RunaValue::Nil);
        }
    }
}
