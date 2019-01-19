mod array_value;
mod midi_event_value;
mod midi_value;
mod num_value;
mod tuple_value;

pub use self::array_value::{ArrayValue, ARRAY_CAPACITY};
pub use self::midi_event_value::MidiEventValue;
pub use self::midi_value::MidiValue;
pub use self::num_value::NumValue;
pub use self::tuple_value::TupleValue;

use crate::mir::{ConstantValue, VarType};
use inkwell::context::Context;
use inkwell::types::{BasicType, StructType};
use inkwell::values::{BasicValue, BasicValueEnum};

pub fn remap_type(context: &Context, mir_type: &VarType) -> StructType {
    match mir_type {
        VarType::Num => NumValue::get_type(context),
        VarType::Midi => MidiValue::get_type(context),
        VarType::Tuple(inner_types) => {
            let inner_structs: Vec<_> =
                inner_types.iter().map(|t| remap_type(context, t)).collect();
            let inner_types: Vec<_> = inner_structs.iter().map(|t| t as &BasicType).collect();
            TupleValue::get_type(context, &inner_types)
        }
        VarType::Array(inner_type) => {
            ArrayValue::get_type(context, remap_type(context, &inner_type))
        }
        VarType::Void => panic!("Void type cannot be remapped")
    }
}

pub fn pass_type_by_val(mir_type: &VarType) -> bool {
    match mir_type {
        VarType::Num => true,
        _ => false,
    }
}

pub fn remap_constant(context: &Context, value: &ConstantValue) -> BasicValueEnum {
    match value {
        ConstantValue::Num(num) => {
            NumValue::get_const(context, num.left, num.right, num.form as u8).into()
        }
        ConstantValue::Tuple(tuple) => {
            let values: Vec<_> = tuple
                .items
                .iter()
                .map(|val| remap_constant(context, val))
                .collect();
            let value_refs: Vec<_> = values.iter().map(|val| val as &BasicValue).collect();
            TupleValue::get_const(context, &value_refs).into()
        }
    }
}
