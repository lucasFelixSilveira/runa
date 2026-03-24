use crate::core::{parser::function, *};

mod string;
mod numeric;
mod table;

pub fn runa_expression(runa: &mut Runa, token: &String) -> (bool, RunaValue) {
    if isstring(token)
    { return string::runa_expression_string(runa, token); }

    if isnumeric(token)
    { return numeric::runa_expression_numeric(runa, token); }

    if isidentifier(token) {
        let value = runa_peek(runa, token);
        if value.is_none()
        { return ( false, RunaValue::Nil ); }

        let local = value.unwrap();
        if let Local::Variable(var) = local
        { return ( true, var.value.clone() ); }

        if let Local::Function(_) = local {
            let paren = lexer::next(runa);
            if paren.as_str().unwrap() != "(" { return ( false, RunaValue::Nil ); }
            _ = function(runa, token);
            let data = runa.return_value.clone().unwrap_or_else(|| RunaValue::Nil);
            runa.return_value = None;
            return ( true, data );
        }
    }

    ( false, RunaValue::Nil )
}
