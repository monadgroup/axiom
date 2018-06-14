use mir::Block;
use mir::block::{Function, Statement};
use ast::ControlField;
use std::fmt;

#[derive(Debug, PartialEq, Eq, Clone)]
pub enum VarType {
    Num,
    Midi,
    Tuple(Vec<VarType>),
    Array(Box<VarType>)
}

impl VarType {
    pub fn of_statement(block: &Block, index: usize) -> VarType {
        match &block.statements[index] {
            Statement::NumConstant { .. } => VarType::Num,
            Statement::NumConvert { .. } => VarType::Num,
            Statement::NumCast { .. } => VarType::Num,
            Statement::NumUnaryOp { .. } => VarType::Num,
            Statement::NumPostfixOp { .. } => VarType::Num,
            Statement::NumMathOp { .. } => VarType::Num,
            Statement::ExtractOp { tuple, index } => {
                if let VarType::Tuple(mut types) = VarType::of_statement(block , *tuple) { types.remove(*index) }
                    else { panic!("Attempted to extract element type of non-tuple") }
            },
            Statement::CallFunc { function, .. } => VarType::of_function(*function),
            Statement::StoreControl { field, .. } => VarType::of_control_field(*field),
            Statement::LoadControl { field, .. } => VarType::of_control_field(*field)
        }
    }

    pub fn of_function(function: Function) -> VarType {
        unimplemented!();
    }

    pub fn of_control_field(field: ControlField) -> VarType {
        unimplemented!();
    }
}

impl fmt::Display for VarType {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            VarType::Num => write!(f, "num"),
            VarType::Midi => write!(f, "midi"),
            VarType::Tuple(ref items) => {
                if let Err(err) = write!(f, "tuple (") { return Err(err); }
                for (index, subtype) in items.iter().enumerate() {
                    if let Err(err) = write!(f, "{:?}", subtype) { return Err(err); }
                    if index < items.len() - 1 {
                        if let Err(err) = write!(f, ", ") { return Err(err); }
                    }
                }
                write!(f, ")")
            },
            VarType::Array(ref subtype) => {
                write!(f, "array [{:?}]", subtype)
            }
        }
    }
}
