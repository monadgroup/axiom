use ast::ControlType;
use codegen::controls::{
    get_data_type, get_ui_type, AudioControl, AudioExtractControl, Control, GraphControl,
    MidiControl, MidiExtractControl, RollControl, ScopeControl,
};
use inkwell::context::Context;
use inkwell::types::{BasicType, StructType};
use mir::block::{Function, Statement};
use mir::Block;
use std::collections::HashMap;

pub struct BlockMetadata {
    block_indexes: HashMap<usize, usize>,
}

pub fn build_block_data_type(context: &Context, block: &Block) -> (StructType, BlockMetadata) {
    let mut result_types = Vec::new();
    let mut block_indexes = HashMap::new();

    for control in &block.controls {
        result_types.push(get_data_type(context, control.control_type));
    }

    for (index, statement) in block.statements.iter().enumerate() {
        if let Statement::CallFunc { function, .. } = statement {
            block_indexes.insert(index, result_types.len());
            result_types.push(build_function_data_type(context, &function));
        }
    }

    let result_type_refs: Vec<_> = result_types.iter().map(|x| x as &BasicType).collect();
    (
        context.struct_type(&result_type_refs, false),
        BlockMetadata { block_indexes },
    )
}

pub fn build_ui_data_type(context: &Context, block: &Block) -> StructType {
    let mut result_types = Vec::new();

    for control in &block.controls {
        result_types.push(get_ui_type(context, control.control_type));
    }

    let result_type_refs: Vec<_> = result_types.iter().map(|x| x as &BasicType).collect();
    context.struct_type(&result_type_refs, false)
}

pub fn build_function_data_type(context: &Context, function: &Function) -> StructType {
    unimplemented!()
}

impl BlockMetadata {
    pub fn control_index(&self, control: usize) -> usize {
        // controls are always the first items in the metadata
        control
    }

    pub fn function_index(&self, statement: usize) -> Option<usize> {
        self.block_indexes.get(&statement).cloned()
    }
}
