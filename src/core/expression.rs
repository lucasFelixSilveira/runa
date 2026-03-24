use crate::core::*;

mod string;
mod numeric;

pub fn runa_expression(runa: &mut Runa, token: &String) -> (bool, RunaValue) {
    if isstring(token)
    { return string::runa_expression_string(runa, token); }

    if isnumeric(token)
    { return numeric::runa_expression_numeric(runa, token); }

    if isidentifier(token) {
        let value = runa_peek(runa, token);
        if value.is_none()
        { return (false, RunaValue::Nil); }

        if let Local::Variable(var) = value.unwrap()
        { return ( true, var.value.clone() ); }
    }

    ( false, RunaValue::Nil )
}
