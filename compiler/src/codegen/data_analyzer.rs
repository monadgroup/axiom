use codegen::TargetProperties;
use codegen::{controls, functions, values};
use inkwell::context::Context;
use inkwell::types::{BasicType, BasicTypeEnum, StructType};
use inkwell::values::{BasicValue, StructValue};
use inkwell::AddressSpace;
use mir::block::{Function, Statement};
use mir::{Block, MIRContext, Node, NodeData, Surface, ValueGroupSource};
use std::collections::HashMap;

#[derive(Debug, Clone)]
pub enum PointerSource {
    Initialized(Vec<usize>),
    Scratch(Vec<usize>),
    Socket(usize),
    Struct(Vec<PointerSource>),
}

#[derive(Debug)]
pub struct NodeLayout {
    pub initialized_const: StructValue,
    pub scratch_struct: StructType,
    pub pointer_struct: StructType,
    pub pointer_sources: Vec<PointerSource>,
}

#[derive(Debug)]
pub struct BlockLayout {
    pub scratch_struct: StructType,
    pub groups_struct: StructType,
    pub group_sources: Vec<usize>,
    pub ui_struct: Option<StructType>,
    pub functions: Vec<Function>,
    control_count: usize,
    func_indexes: HashMap<usize, usize>,
}

#[derive(Debug)]
pub struct SurfaceLayout {
    pub initialized_const: StructValue,
    pub scratch_struct: StructType,
    pub pointer_struct: StructType,
    pub pointer_sources: Vec<PointerSource>,
    group_count: usize,
}

/// Builds up the structure types used for initializing/retaining state of a node.
/// Nodes are made up of several structs:
///
///  - `initialized` is a struct containing pre-initialized data, such as group values. We also keep track
///    of it's initial (constant struct) value.
///  - `scratch` is a struct initialized to zero containing miscellanious data such as node values and groups
///    with no default.
///  - `pointers` is a struct initialized to pointers to other structs. We also keep track of a data structure
///    recording where each entry should point, either to initialized or scratch with a GEP path, or to a
///    socket.
pub fn build_node_layout(
    context: &Context,
    mir: &MIRContext,
    node: &Node,
    target: &TargetProperties,
) -> NodeLayout {
    match node.data {
        NodeData::Custom(ref block_id) => {
            let block_layout = build_block_layout(context, mir.block(block_id).unwrap(), target);

            // a block never has an initialized struct
            let initialized_const = context.const_struct(&[], false);

            // the output scratch type is the block's scratch type + it's UI data, if it exists
            let scratch_struct = if let Some(ui_struct) = block_layout.ui_struct {
                context.struct_type(&[&block_layout.scratch_struct, &ui_struct], false)
            } else {
                context.struct_type(&[&block_layout.scratch_struct], false)
            };

            let pointer_struct = block_layout.groups_struct;
            let pointer_sources: Vec<_> = block_layout
                .group_sources
                .into_iter()
                .map(|socket_index| PointerSource::Socket(socket_index))
                .collect();

            NodeLayout {
                initialized_const,
                scratch_struct,
                pointer_struct,
                pointer_sources,
            }
        }
        NodeData::Group(ref surface_id) => {
            let surface_layout =
                build_surface_layout(context, mir, mir.surface(surface_id).unwrap(), target);

            // the surface mapping is currently just 1:1
            NodeLayout {
                initialized_const: surface_layout.initialized_const,
                scratch_struct: surface_layout.scratch_struct,
                pointer_struct: surface_layout.pointer_struct,
                pointer_sources: surface_layout.pointer_sources,
            }
        }
        NodeData::ExtractGroup { .. } => unimplemented!(),
    }
}

/// Builds up the structure types used for retaining state of a block.
/// Blocks are made up of two or three structs, depending on if we're in the editor or not:
///
///  - `scratch` is a struct that is initialized to zero. It contains runtime data used by controls
///    and functions for persistance between samples.
///  - `groups` is a struct with an entry per control pointing to the group values.
///  - `ui` is a zero-initialized struct similar to `scratch` which contains UI data for controls
///    that use it.
pub fn build_block_layout(
    context: &Context,
    block: &Block,
    target: &TargetProperties,
) -> BlockLayout {
    let mut scratch_types = Vec::new();
    let mut group_types = Vec::new();
    let mut group_sources = Vec::new();
    let mut ui_types = if target.include_ui {
        Some(Vec::new())
    } else {
        None
    };
    let mut functions = Vec::new();
    let mut func_indexes = HashMap::new();

    for (control_index, control) in block.controls.iter().enumerate() {
        scratch_types.push(controls::get_data_type(context, control.control_type));
        group_types.push(
            controls::get_group_type(context, control.control_type).ptr_type(AddressSpace::Generic),
        );
        group_sources.push(control_index);

        if let Some(ref mut ui_types) = ui_types {
            ui_types.push(controls::get_ui_type(context, control.control_type));
        }
    }

    for (index, statement) in block.statements.iter().enumerate() {
        if let Statement::CallFunc { function, .. } = statement {
            functions.push(*function);
            func_indexes.insert(index, scratch_types.len());
            scratch_types.push(functions::get_data_type(context, *function));
        }
    }

    let scratch_type_refs: Vec<_> = scratch_types.iter().map(|x| x as &BasicType).collect();
    let group_type_refs: Vec<_> = group_types.iter().map(|x| x as &BasicType).collect();

    BlockLayout {
        scratch_struct: context.struct_type(&scratch_type_refs, false),
        groups_struct: context.struct_type(&group_type_refs, false),
        group_sources,
        ui_struct: if let Some(ui_types) = ui_types {
            let ui_type_refs: Vec<_> = ui_types.iter().map(|x| x as &BasicType).collect();
            Some(context.struct_type(&ui_type_refs, false))
        } else {
            None
        },
        functions,
        control_count: block.controls.len(),
        func_indexes,
    }
}

/// Builds up the structure types and default values used for initializing/retaining state of a surface.
///
///  - `initialized` is a struct containing pre-initialized value group values.
///  - `scratch` is a struct initialized to zero, containing the scratch data for each node and groups that are initialized to zero.
///  - `pointers` is a struct initialized to pointers to other structs.
pub fn build_surface_layout(
    context: &Context,
    mir: &MIRContext,
    surface: &Surface,
    target: &TargetProperties,
) -> SurfaceLayout {
    let mut initialized_values = Vec::new();
    let mut scratch_types = Vec::new();

    let mut pointer_types = Vec::new();
    let mut pointer_sources = Vec::new();

    let group_pointers: Vec<_> = surface
        .groups
        .iter()
        .map(|group| {
            let value_type = values::remap_type(context, &group.value_type);
            match group.source {
                ValueGroupSource::None => {
                    let scratch_index = scratch_types.len();
                    scratch_types.push(value_type);

                    PointerSource::Scratch(vec![scratch_index])
                }
                ValueGroupSource::Socket(socket_index) => PointerSource::Socket(socket_index),
                ValueGroupSource::Default(ref default_val) => {
                    let initialized_index = initialized_values.len();
                    initialized_values.push(values::remap_constant(context, default_val));

                    PointerSource::Initialized(vec![initialized_index])
                }
            }
        })
        .collect();

    for node in &surface.nodes {
        let layout = build_node_layout(context, mir, node, target);
        let initialized_index = initialized_values.len();
        let scratch_index = scratch_types.len();

        initialized_values.push(layout.initialized_const.into());
        scratch_types.push(layout.scratch_struct);
        pointer_types.push(layout.pointer_struct);
        let new_pointer_source = PointerSource::Struct(
            layout
                .pointer_sources
                .into_iter()
                .map(|original_src| {
                    remap_pointer_source(
                        original_src,
                        node,
                        initialized_index,
                        scratch_index,
                        &group_pointers,
                    )
                })
                .collect(),
        );
        pointer_sources.push(new_pointer_source);
    }

    let initialized_val_refs: Vec<_> = initialized_values
        .iter()
        .map(|x| x as &BasicValue)
        .collect();
    let scratch_type_refs: Vec<_> = scratch_types.iter().map(|x| x as &BasicType).collect();
    let pointer_type_refs: Vec<_> = pointer_types.iter().map(|x| x as &BasicType).collect();

    SurfaceLayout {
        initialized_const: context.const_struct(&initialized_val_refs, false),
        scratch_struct: context.struct_type(&scratch_type_refs, false),
        pointer_struct: context.struct_type(&pointer_type_refs, false),
        pointer_sources,
        group_count: surface.groups.len(),
    }
}

fn remap_pointer_source(
    source: PointerSource,
    node: &Node,
    initialized_index: usize,
    scratch_index: usize,
    pointer_sources: &[PointerSource],
) -> PointerSource {
    match source {
        PointerSource::Initialized(mut indexes) => {
            indexes.insert(0, initialized_index);
            PointerSource::Initialized(indexes)
        }
        PointerSource::Scratch(mut indexes) => {
            indexes.insert(0, scratch_index);
            PointerSource::Scratch(indexes)
        }
        PointerSource::Socket(socket_index) => {
            let socket_group = node.sockets[socket_index].group_id;
            pointer_sources[socket_group].clone()
        }
        PointerSource::Struct(mut sources) => {
            for src in sources.iter_mut() {
                *src = remap_pointer_source(
                    src.clone(),
                    node,
                    initialized_index,
                    scratch_index,
                    pointer_sources,
                )
            }
            PointerSource::Struct(sources)
        }
    }
}

impl NodeLayout {
    pub fn scratch_index(&self) -> usize {
        0
    }

    pub fn ui_index(&self) -> usize {
        1
    }
}

impl BlockLayout {
    pub fn control_index(&self, control: usize) -> usize {
        // controls are always ordered first
        control
    }

    pub fn function_index(&self, function: usize) -> usize {
        self.control_count + function
    }

    pub fn statement_index(&self, statement: usize) -> Option<usize> {
        self.func_indexes.get(&statement).cloned()
    }
}

impl SurfaceLayout {
    pub fn group_index(&self, group: usize) -> usize {
        // groups are always ordered first
        group
    }

    pub fn node_index(&self, node: usize) -> usize {
        self.group_count + node
    }
}
