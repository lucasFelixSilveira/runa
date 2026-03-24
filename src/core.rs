use std::fs::File;
use std::io::BufReader;
use std::process::exit;
pub mod lexer;
pub mod parser;
pub mod expression;

pub type RunaCallback = fn(*mut Runa);

#[derive(Clone, Debug, Default, PartialEq)]
pub enum RunaValue {
    String(String),
    Integer(usize),
    Float(f64),
    Boolean(bool),

    #[default]
    Nil,
}

#[allow(unpredictable_function_pointer_comparisons)]
#[derive(Clone, Debug, Default, PartialEq)]
pub struct Function {
    pub name: String,
    pub std: bool,
    pub argv: Option<Vec<String>>,
    pub argc: usize,
    pub body: Option<String>,
    pub callback: Option<RunaCallback>
}

#[derive(Clone, Debug, Default, PartialEq)]
pub struct Variable {
    pub name: String,
    pub value: RunaValue
}

#[derive(Debug, PartialEq)]
pub enum Local {
    Function(Function),
    Variable(Variable),
}

pub type Locals = Vec<Local>;

#[derive(Debug, Default)]
pub struct Runa {
    pub error: bool,
    pub pushed: Vec<lexer::Token>,
    pub code_stack: Vec<String>,
    pub filename: Option<String>,
    pub file: Option<BufReader<File>>,
    pub stack: Vec<Locals>,
    pub args: Vec<Vec<RunaValue>>,
    pub should_leave: bool,
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

pub fn runa_peek<'a>(runa: &'a Runa, name: &String) -> Option<&'a Local> {
    if runa.stack.len() <= 0 { return None; }
    let locals = runa.stack.last().unwrap();
    for local in locals {
        match local {
            Local::Function(func) if func.name == *name => return Some(&local),
            Local::Variable(var) if var.name == *name => return Some(&local),
            _ => continue
        }
    }
    None
}

pub fn runa_peek_function<'a>(runa: &'a Runa, name: &String) -> Option<&'a Function> {
    let local = runa_peek(runa, name);
    if local.is_none() { runa_spawn_fatal_error(["you tried to call `", name.as_str(), "` but it's unknown"].concat()); }

    if let Local::Function(func) = local? {
        return Some(&func);
    }

    runa_spawn_fatal_error([name.as_str(), " isn't a function"].concat());
    None
}

#[allow(unused)]
pub fn runa_peek_variable<'a>(runa: &'a Runa, name: &String) -> Option<&'a Variable> {
    let local = runa_peek(runa, name);
    if local.is_none() { runa_spawn_fatal_error(["you tried acess `", name.as_str(), "` but it's unknown"].concat()); }

    if let Local::Variable(var) = local? {
        return Some(&var);
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
    }
}

#[allow(unreachable_patterns)]
pub fn runa_assign_local(runa: &mut Runa, name: String, value: RunaValue) {
    if let Some(locals) = runa.stack.last_mut() {
        if let Some(local) = locals.iter_mut().find(|l| {
            match l {
                Local::Variable(var) => var.name == name,
                Local::Function(func) => func.name == name,
                _ => false,
            }
        }) {
            match local {
                Local::Variable(var) => var.value = value,
                _ => {}
            }
            return;
        }
    }
    runa_push_local(runa, Local::Variable(Variable { name, value }));
}

pub fn parse(runa: &mut Runa) {
    parser::parse(runa);
}
