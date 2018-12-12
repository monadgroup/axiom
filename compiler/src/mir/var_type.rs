use crate::ast::{
    AudioExtractField, AudioField, ControlField, ControlType, GraphField, MidiExtractField,
    MidiField, RollField, ScopeField,
};
use crate::mir::block::{Function, Statement};
use crate::mir::Block;
use crate::mir::ConstantValue;
use std::fmt;

#[derive(Debug, PartialEq, Eq, Clone)]
pub enum VarType {
    Num,
    Midi,
    Tuple(Vec<VarType>),
    Array(Box<VarType>),
}

impl VarType {
    pub fn new_array(sub_type: VarType) -> VarType {
        VarType::Array(Box::new(sub_type))
    }

    pub fn of_constant(constant: &ConstantValue) -> VarType {
        match constant {
            ConstantValue::Num(_) => VarType::Num,
            ConstantValue::Tuple(tuple) => VarType::Tuple(
                tuple
                    .items
                    .iter()
                    .map(|const_val| VarType::of_constant(const_val))
                    .collect(),
            ),
        }
    }

    pub fn of_statement(block: &Block, index: usize) -> VarType {
        match &block.statements[index] {
            Statement::Constant(ref constant) => VarType::of_constant(constant),
            Statement::Global(_) => VarType::Num, // review: some globals might not be num?
            Statement::NumConvert { .. } => VarType::Num,
            Statement::NumCast { .. } => VarType::Num,
            Statement::NumUnaryOp { .. } => VarType::Num,
            Statement::NumMathOp { .. } => VarType::Num,
            Statement::Extract { tuple, index } => {
                if let VarType::Tuple(mut types) = VarType::of_statement(block, *tuple) {
                    types.remove(*index)
                } else {
                    panic!("Attempted to extract element type of non-tuple")
                }
            }
            Statement::Combine { indexes } => VarType::Tuple(
                indexes
                    .iter()
                    .map(|index| VarType::of_statement(block, *index))
                    .collect(),
            ),
            Statement::CallFunc { function, .. } => VarType::of_function(*function),
            Statement::StoreControl { field, .. } => VarType::of_control_field(*field),
            Statement::LoadControl { field, .. } => VarType::of_control_field(*field),
        }
    }

    pub fn of_function(function: Function) -> VarType {
        function.return_type()
    }

    pub fn of_control_value(control: ControlType) -> VarType {
        match control {
            ControlType::Audio => VarType::Num,
            ControlType::Graph => VarType::Num,
            ControlType::Midi => VarType::Midi,
            ControlType::Roll => VarType::Midi,
            ControlType::Scope => VarType::Num,
            ControlType::AudioExtract => VarType::new_array(VarType::Num),
            ControlType::MidiExtract => VarType::new_array(VarType::Midi),
        }
    }

    pub fn of_control_field(field: ControlField) -> VarType {
        match field {
            ControlField::Audio(AudioField::Value) => VarType::Num,
            ControlField::Graph(GraphField::Value) => VarType::Num,
            ControlField::Graph(GraphField::State) => VarType::Num,
            ControlField::Graph(GraphField::Paused) => VarType::Num,
            ControlField::Graph(GraphField::Time) => VarType::Num,
            ControlField::Midi(MidiField::Value) => VarType::Midi,
            ControlField::Roll(RollField::Value) => VarType::Midi,
            ControlField::Roll(RollField::Speed) => VarType::Num,
            ControlField::Scope(ScopeField::Value) => VarType::Num,
            ControlField::AudioExtract(AudioExtractField::Value) => {
                VarType::new_array(VarType::Num)
            }
            ControlField::MidiExtract(MidiExtractField::Value) => VarType::new_array(VarType::Midi),
        }
    }

    pub fn base_type(&self) -> Option<&VarType> {
        match self {
            VarType::Array(base) => Some(base.as_ref()),
            _ => None,
        }
    }
}

impl fmt::Display for VarType {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            VarType::Num => write!(f, "num"),
            VarType::Midi => write!(f, "midi"),
            VarType::Tuple(ref items) => {
                write!(f, "tuple(")?;
                for (index, subtype) in items.iter().enumerate() {
                    write!(f, "{:?}", subtype)?;
                    if index < items.len() - 1 {
                        write!(f, ", ")?;
                    }
                }
                write!(f, ")")
            }
            VarType::Array(ref subtype) => write!(f, "array [{:?}]", subtype),
        }
    }
}
