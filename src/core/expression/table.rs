use crate::core::{expression::runa_expression, *};

pub fn runa_expression_tables(runa: &mut Runa, _: &String) -> (bool, RunaValue) {
    let next = lexer::next(runa);
    if next.is_eof()
    { return ( false, RunaValue::Nil ); }

    if next.as_str().unwrap() == "}"
    { return ( true, RunaValue::Table(Vec::new()) ); }

    lexer::back(runa, next);

    let mut i = 1;
    let mut table = Vec::new();
    loop {
        let next  = lexer::next(runa);
        if next.is_eof() { return ( false, RunaValue::Nil ); }
        if next.as_str().unwrap() == "}" {
            lexer::back(runa, next);
            return ( true, RunaValue::Table(table) );
        }

        let after = lexer::next(runa);
        if after.is_eof() { return ( false, RunaValue::Nil ); }

        if after.as_str().unwrap() == "=" {
            let key = next.as_str().unwrap().to_string();
            let resolve = lexer::next(runa);
            if resolve.is_eof() { return ( false, RunaValue::Nil ); }

            let (success, value) = runa_expression(runa, resolve.as_str().unwrap());
            if !success { return ( false, RunaValue::Nil ); }
            let rc = Rc::new(RefCell::new(value));
            table.push((key, rc));

            let comma = lexer::next(runa);
            if comma.is_eof() { return ( false, RunaValue::Nil ); }
            if comma.as_str().unwrap() != "," { return ( false, RunaValue::Nil ); }
            continue;
        }

        lexer::back(runa, after);

        let (success, value) = runa_expression(runa, next.as_str().unwrap());
        if !success { return ( false, RunaValue::Nil ); }

        let after = lexer::next(runa);

        let rc = Rc::new(RefCell::new(value));
        table.push((i.to_string(), rc));
        i += 1;

        if after.is_eof() { return ( false, RunaValue::Nil ); }
        if after.as_str().unwrap() == "," { continue; }
        if after.as_str().unwrap() == "}" { break; }
        return ( false, RunaValue::Nil );
    }

    return ( true, RunaValue::Table(table) );
}
