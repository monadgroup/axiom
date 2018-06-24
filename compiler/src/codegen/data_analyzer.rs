use codegen::controls::{get_data_type, get_group_type, get_ui_type};
use inkwell::context::Context;
use inkwell::types::{BasicType, StructType};
use inkwell::AddressSpace;
use mir::block::{Function, Statement};
use mir::Block;
use std::collections::HashMap;

pub struct BlockLayout {
    pub scratch_struct: StructType,
    pub groups_struct: StructType,
    pub ui_struct: Option<StructType>,
    func_indexes: HashMap<usize, usize>,
}

/// Builds up the structure types used for retaining state of a block.
/// Blocks are made up of two or three structs, depending on if we're in the editor or not:
///
///  - `scratch` is a struct that is initialized to zero. It contains runtime data used by controls
///    and functions for persistance between samples.
///  - `groups` is a struct with an entry per control pointing to the group values.
///  - `ui` is a zero-initialized struct similar to `scratch` which contains UI data for controls
///    that use it.
pub fn build_block_layout(context: &Context, block: &Block, include_ui: bool) -> BlockLayout {
    let mut scratch_types = Vec::new();
    let mut group_types = Vec::new();
    let mut ui_types = if include_ui { Some(Vec::new()) } else { None };
    let mut func_indexes = HashMap::new();

    for control in &block.controls {
        scratch_types.push(get_data_type(context, control.control_type));
        group_types
            .push(get_group_type(context, control.control_type).ptr_type(AddressSpace::Generic));

        if let Some(ref mut ui_types) = ui_types {
            ui_types.push(get_ui_type(context, control.control_type));
        }
    }

    for (index, statement) in block.statements.iter().enumerate() {
        if let Statement::CallFunc { function, .. } = statement {
            func_indexes.insert(index, scratch_types.len());
            scratch_types.push(build_function_data_type(context, &function));
        }
    }

    let scratch_type_refs: Vec<_> = scratch_types.iter().map(|x| x as &BasicType).collect();
    let group_type_refs: Vec<_> = group_types.iter().map(|x| x as &BasicType).collect();

    BlockLayout {
        scratch_struct: context.struct_type(&scratch_type_refs, false),
        groups_struct: context.struct_type(&group_type_refs, false),
        ui_struct: if let Some(ui_types) = ui_types {
            let ui_type_refs: Vec<_> = ui_types.iter().map(|x| x as &BasicType).collect();
            Some(context.struct_type(&ui_type_refs, false))
        } else {
            None
        },
        func_indexes,
    }
}

pub fn build_function_data_type(_context: &Context, _function: &Function) -> StructType {
    unimplemented!()
}

impl BlockLayout {
    pub fn control_index(&self, control: usize) -> usize {
        // controls are always ordered first
        control
    }

    pub fn function_index(&self, statement: usize) -> Option<usize> {
        self.func_indexes.get(&statement).cloned()
    }
}
