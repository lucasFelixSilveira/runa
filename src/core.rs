use std::cell::RefCell;
use std::ffi::*;
use std::fs::File;
use std::io::BufReader;
use std::rc::Rc;
use std::process::exit;
pub mod lexer;
pub mod parser;
pub mod expression;
pub mod internal;
pub mod statements;
pub mod lzma;

pub type RunaCallback = fn(*mut Runa);

#[derive(Clone, Debug, Default, PartialEq)]
pub enum RunaValue {
    String(String),
    Integer(isize),
    Float(f64),
    Boolean(bool),
    Table(String, Vec<(String, Rc<RefCell<RunaValue>>)>),

    /*
     * this is a special type to Runa use in Tables.
     * It just make some global value reference using
     * a `String` as the identifier
     */
    Pointer(String),

    #[default]
    Nil,
}

impl RunaValue {
    pub fn kind_to_string(&self) -> &str {
        match self {
            RunaValue::String(_) => "string",
            RunaValue::Integer(_) => "integer",
            RunaValue::Float(_) => "float",
            RunaValue::Boolean(_) => "boolean",
            RunaValue::Table(_, _) => "table",
            RunaValue::Pointer(_) => "pointer",
            RunaValue::Nil => "nil",
        }
    }
}

#[allow(unpredictable_function_pointer_comparisons)]
#[derive(Clone, Debug, Default, PartialEq)]
pub struct Function {
    pub name: String,
    pub std: bool,
    pub argv: Option<Vec<String>>,
    pub argc: usize,
    pub body: Option<Vec<u8>>,
    pub callback: Option<RunaCallback>
}

#[derive(Clone, Debug, Default, PartialEq)]
pub struct Variable {
    pub name: String,
    pub value: RunaValue
}

#[derive(Clone, Debug, PartialEq)]
pub enum Local {
    Function(Function),
    Variable(Variable),
}

pub type Locals = Vec<Local>;

#[derive(Debug, Default)]
pub struct Runa {
    pub error: bool,
    pub pushed: Vec<lexer::Token>,
    pub filename: Option<String>,
    pub file: Option<BufReader<File>>,
    pub stack: Vec<Locals>,
    pub args: Vec<Vec<RunaValue>>,
    pub should_leave: bool,
    pub return_value: Option<RunaValue>,
    pub modularization: Vec<i32>,
    pub if_resolved: Vec<bool>,
    pub bodies: Vec<String>,
    pub fields: Vec<(String, RunaValue)>,
}

pub fn runa_spawn_fatal_error(message: String) {
    println!("[Runa] [Fatal]: {}", message);
    exit(1);
}

pub fn isidentifier(token: &str) -> bool {
    if token.is_empty() { return false; }
    let first = token.chars().next().unwrap();
    if !first.is_alphabetic() && first != '_' { return false; }
    for c in token.chars().skip(1) {
        if !c.is_alphanumeric() && c != '_' { return false; }
    }
    true
}

fn isinteger(s: &str) -> bool {
    s.parse::<i64>().is_ok() && !s.contains('.')
}
fn isfloat(s: &str) -> bool {
    s.parse::<f64>().is_ok() && s.contains('.')
}

pub fn isnumeric(token: &str) -> bool {
    isinteger(token) || isfloat(token)
}

pub fn isstring(token: &str) -> bool {
    ( token.starts_with('"') && token.ends_with('"') ) || ( token.starts_with('\'') && token.ends_with('\'') )
}

pub fn runa_peek(runa: &Runa, name: &String) -> Option<Local> {
    if runa.stack.is_empty() { return None; }

    for i in (0..runa.stack.len()).rev() {
        let locals = runa.stack.get(i).unwrap();
        for local in locals {
            match local {
                Local::Function(func) if func.name == *name => return Some(local.clone()),
                Local::Variable(var) if var.name == *name => return Some(local.clone()),
                _ => continue
            }
        }
    }

    None
}

pub fn runa_peek_mutable(runa: &mut Runa, name: String) -> Option<&mut Local> {
    for locals in runa.stack.iter_mut().rev() {
        for local in locals.iter_mut() {
            match local {
                Local::Function(func) if func.name == name => return Some(local),
                Local::Variable(var) if var.name == name => return Some(local),
                _ => {}
            }
        }
    }

    None
}

pub fn runa_peek_function(runa: &Runa, name: &String) -> Option<Function> {
    let local = runa_peek(runa, name);
    if local.is_none() { runa_spawn_fatal_error(["you tried to call `", name.as_str(), "` but it's unknown"].concat()); }

    if let Local::Function(func) = local? {
        return Some(func);
    }

    runa_spawn_fatal_error([name.as_str(), " isn't a function"].concat());
    None
}

#[allow(unused)]
pub fn runa_peek_variable(runa: &Runa, name: &String) -> Option<Variable> {
    let local = runa_peek(runa, name);
    if local.is_none() { runa_spawn_fatal_error(["you tried acess `", name.as_str(), "` but it's unknown"].concat()); }

    if let Local::Variable(var) = local? {
        return Some(var);
    }

    runa_spawn_fatal_error([name.as_str(), " isn't a variable"].concat());
    None
}

pub fn runa_push_local(runa: &mut Runa, local: Local) {
    if runa.stack.len() <= 0 {
        runa.stack.push(Vec::new());
    }
    let locals = runa.stack.last_mut().unwrap();
    locals.push(local);
}

pub fn runa_value_to_string(value: &RunaValue) -> String {
    match value {
        RunaValue::String(s) => s.clone(),
        RunaValue::Integer(i) => i.to_string(),
        RunaValue::Float(f) => f.to_string(),
        RunaValue::Boolean(b) => b.to_string(),
        RunaValue::Nil => "nil".to_string(),
        RunaValue::Pointer(addr) => format!("pointer to runa:{addr}"),
        RunaValue::Table(id, table) => {
            let mut s = format!("table({}...{}) {{ ", id[..3].to_string(), id[24..].to_string());
            for (name, value) in table
            { s.push_str(&format!("{} = {}, ", name, runa_value_to_string(&*value.borrow()))); }
            s.push('}');
            s
        }
    }
}

#[allow(unreachable_patterns)]
pub fn runa_assign_local(runa: &mut Runa, name: String, value: RunaValue) {
    let from_stack = runa_peek_mutable(runa, name.clone());
    if from_stack.is_none()
    { runa_spawn_fatal_error(format!("You are trying to make a redefinition by `{name}`, a invalid variable")); }

    if let Local::Function(_) = from_stack.as_ref().unwrap()
    { runa_spawn_fatal_error(format!("You can't redefine `{name}`, because it's a function.")); }

    let Local::Variable(var) = from_stack.unwrap() else { unreachable!() };
    (*var).value = value;
}

pub fn runa_peek_value_to_pointer(runa: &Runa, val: RunaValue) -> RunaValue {
    let RunaValue::Pointer(addr) = &val else {
        return val;
    };

    let variable = runa_peek_variable(runa, &format!("runa:{addr}"));
    let Some(Variable { name, value }) = variable else {
        runa_spawn_fatal_error(format!("Can't access the pointer {addr}"));
        unreachable!()
    };

    _ = name;
    return value
}

pub fn runa_peek_table_by_internal_id(runa: &Runa, internal_id: *const u8) -> &RunaValue {
    if internal_id.is_null() { return &RunaValue::Nil; }
    let c_str = unsafe { CStr::from_ptr(internal_id as *const i8) };
    let id_str = match c_str.to_str() {
        Ok(s) => s,
        Err(_) => return &RunaValue::Nil,
    };

    let locals = match runa.stack.last() {
        Some(locals) => locals,
        None => return &RunaValue::Nil,
    };

    for local in locals {
        if let Local::Variable(var) = local {
            if let RunaValue::Table(table_id, _) = &var.value {
                if [table_id, "0"].concat() == id_str {
                    return &var.value;
                }
            }
        }
    }

    &RunaValue::Nil
}

pub fn parse(runa: &mut Runa) {
    parser::parse(runa);
}
