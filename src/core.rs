
use std::fs::File;
use std::io::BufReader;
use std::process::exit;
pub mod lexer;
pub mod parser;
pub mod expression;

pub type RunaCallback = fn(*mut Runa);

#[allow(unpredictable_function_pointer_comparisons)]
#[derive(Debug, Default, PartialEq)]
pub struct Function {
    pub name: String,
    pub std: bool,
    pub argv: Option<Vec<String>>,
    pub argc: usize,
    pub body: Option<String>,
    pub callback: Option<RunaCallback>
}

#[derive(Debug, PartialEq)]
pub enum Local {
    Function(Function)
}

pub type Locals = Vec<Local>;

#[derive(Debug, PartialEq)]
pub enum RunaValue {
    String(String),
    Integer(usize),
    Float(f64),
    Boolean(bool),
    Nil
}

#[derive(Debug, Default)]
pub struct Runa {
    pub error: bool,
    pub pushed: Option<lexer::Token>,
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

pub fn isidentifier(token: &String) -> bool {
    if token.is_empty() { return false; }
    let first = token.chars().next().unwrap();
    if !first.is_alphabetic() && first != '_' { return false; }
    for c in token.chars().skip(1) {
        if !c.is_alphanumeric() && c != '_' { return false; }
    }
    true
}

pub fn runa_peek<'a>(runa: &'a Runa, name: &String) -> Option<&'a Local> {
    if runa.stack.len() <= 0 { return None; }
    let locals = runa.stack.last().unwrap();
    for local in locals {
        match local {
            Local::Function(func) if func.name == *name => return Some(&local),
            _ => todo!()
        }
    }
    None
}

#[allow(irrefutable_let_patterns)]
pub fn runa_peek_function<'a>(runa: &'a Runa, name: &String) -> Option<&'a Function> {
    let local = runa_peek(runa, name);
    if local.is_none() { runa_spawn_fatal_error(["you tried to call `", name.as_str(), "` but it's unknown"].concat()); }

    if let Local::Function(func) = local? {
        return Some(&func);
    }

    runa_spawn_fatal_error([name.as_str(), " isn't a function"].concat());
    None
}

pub fn runa_push_local(runa: &mut Runa, local: Local) {
    if runa.stack.len() <= 0 {
        runa.stack.push(Vec::new());
    }
    let locals = runa.stack.last_mut().unwrap();
    locals.push(local);
}

pub fn parse(runa: &mut Runa) {
    parser::parse(runa);
}
