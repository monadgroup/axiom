mod array_value;
mod midi_event_value;
mod midi_value;
mod num_value;
mod tuple_value;

pub use self::array_value::ArrayValue;
pub use self::midi_event_value::MidiEventValue;
pub use self::midi_value::MidiValue;
pub use self::num_value::NumValue;
pub use self::tuple_value::TupleValue;

use inkwell::context::Context;
use inkwell::types::{BasicType, StructType};
use mir::VarType;

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
    }
}
