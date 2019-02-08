use crate::codegen::TargetProperties;
use crate::codegen::{controls, functions, values, ObjectCache};
use crate::mir::block::{Function, Statement};
use crate::mir::{Block, Node, NodeData, Surface, ValueGroup, ValueGroupSource};
use inkwell::context::Context;
use inkwell::types::{BasicType, BasicTypeEnum, StructType};
use inkwell::values::{BasicValue, StructValue};
use inkwell::AddressSpace;
use std::collections::HashMap;
use std::{fmt, iter};

#[derive(Debug, Clone, Copy)]
pub enum PointerSourceAggregateType {
    Struct,
    Array,
}

#[derive(Clone)]
pub enum PointerSource {
    Initialized(Vec<usize>),
    Scratch(Vec<usize>),
    Shared(Vec<usize>),
    Socket(usize, Vec<usize>),
    Aggregate(PointerSourceAggregateType, Vec<PointerSource>),
}

impl fmt::Debug for PointerSource {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            PointerSource::Initialized(path) => write!(f, "Initialized{:?}", path),
            PointerSource::Scratch(path) => write!(f, "Scratch{:?}", path),
            PointerSource::Shared(path) => write!(f, "Shared{:?}", path),
            PointerSource::Socket(base, path) => write!(f, "Socket {}{:?}", base, path),
            PointerSource::Aggregate(aggregate_type, items) => {
                write!(f, "{:?}{:?}", aggregate_type, items)
            }
        }
    }
}

#[derive(Debug, Clone)]
pub struct NodeLayout {
    pub initialized_const: StructValue,
    pub scratch_struct: BasicTypeEnum,
    pub shared_struct: BasicTypeEnum,
    pub pointer_struct: StructType,
    pub pointer_sources: Vec<PointerSource>,
}

#[derive(Debug, Clone)]
pub struct BlockLayout {
    pub scratch_struct: StructType,
    pub shared_struct: StructType,
    pub pointer_struct: StructType,
    pub pointer_sources: Vec<PointerSource>,
    pub constant_struct: StructType,
    pub functions: Vec<Function>,
    control_count: usize,
    func_indexes: HashMap<usize, usize>,
}

#[derive(Debug, Clone)]
pub struct SurfaceLayout {
    pub initialized_const: StructValue,
    pub scratch_struct: StructType,
    pub shared_struct: StructType,
    pub pointer_struct: StructType,
    pub pointer_sources: Vec<PointerSource>,
    pub node_layouts: Vec<NodeLayout>,
    node_scratch_offset: usize,
    node_initializer_offset: usize,
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
    cache: &ObjectCache,
    node: &Node,
    parent_groups: &[ValueGroup],
) -> NodeLayout {
    let context = cache.context();

    match node.data {
        NodeData::Dummy => NodeLayout {
            initialized_const: context.const_struct(&[], false),
            scratch_struct: context.struct_type(&[], false).into(),
            shared_struct: context.struct_type(&[], false).into(),
            pointer_struct: context.struct_type(&[], false),
            pointer_sources: Vec::new(),
        },
        NodeData::Custom {
            block,
            ref control_initializers,
        } => {
            let block_layout = cache.block_layout(block).unwrap();

            // build an initialized struct from the control initializers
            let block_mir = cache.block_mir(block).unwrap();

            // No way to collect into multiple vectors at once, so we do this the manual way
            let mut initializer_consts = Vec::new();
            initializer_consts.reserve(control_initializers.len());
            let mut initializer_sources = Vec::new();
            initializer_sources.reserve(control_initializers.len());

            for (initializer, control) in control_initializers.iter().zip(block_mir.controls.iter())
            {
                let (initializer_const, ptr_sources) = controls::get_constant_value(
                    context,
                    control.control_type,
                    initializer,
                    cache.target(),
                );

                let initializer_index = initializer_consts.len();
                initializer_consts.push(initializer_const);
                initializer_sources.push(PointerSource::Aggregate(
                    PointerSourceAggregateType::Struct,
                    map_pointer_sources(
                        ptr_sources,
                        |mut indices| {
                            indices.insert(0, initializer_index);
                            PointerSource::Initialized(indices)
                        },
                        PointerSource::Scratch,
                        PointerSource::Shared,
                        PointerSource::Socket,
                    ),
                ));
            }

            let initializer_refs: Vec<_> = initializer_consts
                .iter()
                .map(|val| val as &BasicValue)
                .collect();
            let initialized_const = context.const_struct(&initializer_refs, false);

            let initializer_source =
                PointerSource::Aggregate(PointerSourceAggregateType::Struct, initializer_sources);

            let pointer_struct = context.struct_type(
                &[&block_layout.constant_struct, &block_layout.pointer_struct],
                false,
            );
            let pointer_sources = vec![
                initializer_source,
                PointerSource::Aggregate(
                    PointerSourceAggregateType::Struct,
                    block_layout.pointer_sources.clone(),
                ),
            ];

            NodeLayout {
                initialized_const,
                scratch_struct: block_layout.scratch_struct.into(),
                shared_struct: block_layout.shared_struct.into(),
                pointer_struct,
                pointer_sources,
            }
        }
        NodeData::Group(surface_id) => {
            let surface_layout = cache.surface_layout(surface_id).unwrap();

            // terminate the shared data here and move it into scratch
            let new_scratch = context.struct_type(
                &[
                    &surface_layout.scratch_struct,
                    &surface_layout.shared_struct,
                ],
                false,
            );

            let new_pointer_sources = map_pointer_sources(
                surface_layout.pointer_sources.iter().cloned(),
                PointerSource::Initialized,
                |mut indices| {
                    indices.insert(0, 0);
                    PointerSource::Scratch(indices)
                },
                |mut indices| {
                    indices.insert(0, 1);
                    PointerSource::Scratch(indices)
                },
                PointerSource::Socket,
            );

            NodeLayout {
                initialized_const: surface_layout.initialized_const,
                scratch_struct: new_scratch.into(),
                shared_struct: context.struct_type(&[], false).into(),
                pointer_struct: surface_layout.pointer_struct,
                pointer_sources: new_pointer_sources,
            }
        }
        NodeData::ExtractGroup {
            surface,
            ref source_sockets,
            ref dest_sockets,
        } => {
            let surface_layout = cache.surface_layout(surface).unwrap();

            // generate new pointer sources that point to each voice
            let voice_pointer_sources: Vec<_> = iter::repeat(&surface_layout.pointer_sources)
                .take(values::ARRAY_CAPACITY as usize)
                .enumerate()
                .map(|(voice_index, pointer_sources)| {
                    let sub_sources = pointer_sources
                        .iter()
                        .map(move |source| {
                            map_extract_pointer_source(
                                source.clone(),
                                voice_index,
                                source_sockets,
                                dest_sockets,
                            )
                        })
                        .collect();
                    PointerSource::Aggregate(PointerSourceAggregateType::Struct, sub_sources)
                })
                .collect();

            // The extract group also needs access to the source and destination arrays.
            // Note: we put the underlying surface's pointers first, as this enables value
            // read-back to read the first instance without any special behavior.
            // For the editor, we also want a pointer to the actual active state of the surface,
            // which we'll put in the scratch.
            //
            // This array must match the struct defined below as `pointer_struct`.
            let pointer_sources = vec![
                PointerSource::Aggregate(
                    PointerSourceAggregateType::Array,
                    map_pointer_sources(
                        voice_pointer_sources,
                        PointerSource::Initialized,
                        |mut indices| {
                            indices.insert(0, 0);
                            PointerSource::Scratch(indices)
                        },
                        PointerSource::Shared,
                        PointerSource::Socket,
                    ),
                ),
                PointerSource::Aggregate(
                    PointerSourceAggregateType::Struct,
                    source_sockets
                        .iter()
                        .map(|socket| PointerSource::Socket(*socket, vec![]))
                        .collect(),
                ),
                PointerSource::Aggregate(
                    PointerSourceAggregateType::Struct,
                    dest_sockets
                        .iter()
                        .map(|socket| PointerSource::Socket(*socket, vec![]))
                        .collect(),
                ),
                PointerSource::Scratch(vec![1]),
            ];

            let source_socket_types: Vec<_> = source_sockets
                .iter()
                .map(|socket| {
                    let socket_group = node.sockets[*socket].group_id;
                    values::remap_type(context, &parent_groups[socket_group].value_type)
                        .ptr_type(AddressSpace::Generic)
                })
                .collect();
            let source_type_refs: Vec<_> = source_socket_types
                .iter()
                .map(|ptr_type| ptr_type as &BasicType)
                .collect();

            let dest_socket_types: Vec<_> = dest_sockets
                .iter()
                .map(|socket| {
                    let socket_group = node.sockets[*socket].group_id;
                    values::remap_type(context, &parent_groups[socket_group].value_type)
                        .ptr_type(AddressSpace::Generic)
                })
                .collect();
            let dest_type_refs: Vec<_> = dest_socket_types
                .iter()
                .map(|ptr_type| ptr_type as &BasicType)
                .collect();

            let scratch_struct = context.struct_type(
                &[
                    &surface_layout
                        .scratch_struct
                        .array_type(u32::from(values::ARRAY_CAPACITY)),
                    &context.i32_type(),
                ],
                false,
            );

            let pointer_struct = context.struct_type(
                &[
                    &surface_layout
                        .pointer_struct
                        .array_type(u32::from(values::ARRAY_CAPACITY))
                        as &BasicType,
                    &context.struct_type(&source_type_refs, false) as &BasicType,
                    &context.struct_type(&dest_type_refs, false) as &BasicType,
                    &context.i32_type().ptr_type(AddressSpace::Generic),
                ],
                false,
            );

            // Each struct is duplicated by the number of items in an array type, except for
            // pre-initialized data (since it's immutable) and shared data (since it's shared).
            NodeLayout {
                initialized_const: surface_layout.initialized_const,
                shared_struct: surface_layout.shared_struct.into(),
                scratch_struct: scratch_struct.into(),
                pointer_struct,
                pointer_sources,
            }
        }
    }
}

fn map_extract_pointer_source(
    source: PointerSource,
    voice_index: usize,
    source_sockets: &[usize],
    destination_sockets: &[usize],
) -> PointerSource {
    match source {
        PointerSource::Initialized(indexes) => {
            // initialized data doesn't need to be copied since it's immutable
            PointerSource::Initialized(indexes)
        }
        PointerSource::Scratch(mut indexes) => {
            // scratch is copied for each voice instance
            indexes.insert(0, voice_index);
            PointerSource::Scratch(indexes)
        }
        PointerSource::Shared(indexes) => {
            // shared data isn't copied since it's shared (yes...)
            PointerSource::Shared(indexes)
        }
        PointerSource::Socket(socket_index, mut sub_indices) => {
            // if the socket index is one of the sources/destinations, we want to get a certain
            // index in it, otherwise we just want the whole thing
            if source_sockets.contains(&socket_index) || destination_sockets.contains(&socket_index)
            {
                // arrays are a struct in the form {bitmap, items} - we only care about items here
                sub_indices.insert(0, 1);
                sub_indices.insert(1, voice_index);
            }

            PointerSource::Socket(socket_index, sub_indices)
        }
        PointerSource::Aggregate(aggregate_type, mut sources) => {
            for src in sources.iter_mut() {
                *src = map_extract_pointer_source(
                    src.clone(),
                    voice_index,
                    source_sockets,
                    destination_sockets,
                )
            }
            PointerSource::Aggregate(aggregate_type, sources)
        }
    }
}

/// Builds up the structure types used for retaining state of a block.
/// Pointers is:
///  - Controls
///     - Value ptr (points to socket)
///     - Data ptr  (points to scratch)
///     - UI ptr    (points to scratch)
///  - Functions
///     - Data (points to scratch)
pub fn build_block_layout(
    context: &Context,
    block: &Block,
    target: &TargetProperties,
) -> BlockLayout {
    let mut scratch_types = Vec::new();
    let mut shared_types = Vec::new();
    let mut pointer_types: Vec<BasicTypeEnum> = Vec::new();
    let mut pointer_sources = Vec::new();
    let mut constant_types = Vec::new();
    let mut functions = Vec::new();
    let mut func_indexes = HashMap::new();

    for (control_index, control) in block.controls.iter().enumerate() {
        let data_index = scratch_types.len();
        let data_type = controls::get_data_type(context, control.control_type);
        scratch_types.push(data_type);

        let shared_index = shared_types.len();
        let shared_type = controls::get_shared_data_type(context, control.control_type);
        shared_types.push(shared_type);

        constant_types.push(controls::get_constant_ptr_type(
            context,
            control.control_type,
        ));

        if target.include_ui {
            let ui_type = controls::get_ui_type(context, control.control_type);
            shared_types.push(ui_type);

            pointer_sources.push(PointerSource::Aggregate(
                PointerSourceAggregateType::Struct,
                vec![
                    PointerSource::Socket(control_index, Vec::new()),
                    PointerSource::Scratch(vec![data_index]),
                    PointerSource::Shared(vec![shared_index]),
                    PointerSource::Shared(vec![shared_index + 1]),
                ],
            ));
            pointer_types.push(
                context
                    .struct_type(
                        &[
                            &controls::get_group_type(context, control.control_type)
                                .ptr_type(AddressSpace::Generic),
                            &data_type.ptr_type(AddressSpace::Generic),
                            &shared_type.ptr_type(AddressSpace::Generic),
                            &ui_type.ptr_type(AddressSpace::Generic),
                        ],
                        false,
                    )
                    .into(),
            );
        } else {
            pointer_sources.push(PointerSource::Aggregate(
                PointerSourceAggregateType::Struct,
                vec![
                    PointerSource::Socket(control_index, Vec::new()),
                    PointerSource::Scratch(vec![data_index]),
                    PointerSource::Shared(vec![shared_index]),
                ],
            ));
            pointer_types.push(
                context
                    .struct_type(
                        &[
                            &controls::get_group_type(context, control.control_type)
                                .ptr_type(AddressSpace::Generic),
                            &data_type.ptr_type(AddressSpace::Generic),
                            &shared_type.ptr_type(AddressSpace::Generic),
                        ],
                        false,
                    )
                    .into(),
            );
        }
    }

    for (index, statement) in block.statements.iter().enumerate() {
        if let Statement::CallFunc { function, .. } = statement {
            functions.push(*function);
            func_indexes.insert(index, pointer_sources.len());
            let func_type = functions::get_data_type(context, *function);
            let scratch_index = scratch_types.len();
            scratch_types.push(func_type);
            pointer_sources.push(PointerSource::Scratch(vec![scratch_index]));
            pointer_types.push(func_type.ptr_type(AddressSpace::Generic).into());
        }
    }

    let scratch_type_refs: Vec<_> = scratch_types.iter().map(|x| x as &BasicType).collect();
    let shared_type_refs: Vec<_> = shared_types.iter().map(|x| x as &BasicType).collect();
    let pointer_type_refs: Vec<_> = pointer_types.iter().map(|x| x as &BasicType).collect();
    let constant_type_refs: Vec<_> = constant_types.iter().map(|x| x as &BasicType).collect();

    BlockLayout {
        scratch_struct: context.struct_type(&scratch_type_refs, false),
        shared_struct: context.struct_type(&shared_type_refs, false),
        pointer_struct: context.struct_type(&pointer_type_refs, false),
        pointer_sources,
        constant_struct: context.struct_type(&constant_type_refs, false),
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
///
pub fn build_surface_layout(cache: &ObjectCache, surface: &Surface) -> SurfaceLayout {
    let context = cache.context();

    let mut initialized_values = Vec::new();
    let mut scratch_types: Vec<BasicTypeEnum> = Vec::new();
    let mut shared_types: Vec<BasicTypeEnum> = Vec::new();

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
                    scratch_types.push(value_type.into());

                    PointerSource::Scratch(vec![scratch_index])
                }
                ValueGroupSource::Socket(socket_index) => {
                    PointerSource::Socket(socket_index, vec![])
                }
                ValueGroupSource::Default(ref default_val) => {
                    let initialized_index = initialized_values.len();
                    initialized_values.push(values::remap_constant(context, default_val));

                    PointerSource::Initialized(vec![initialized_index])
                }
            }
        })
        .collect();

    let node_scratch_offset = scratch_types.len();
    let node_initializer_offset = initialized_values.len();

    let node_layouts: Vec<_> = surface
        .nodes
        .iter()
        .map(|node| build_node_layout(cache, node, &surface.groups))
        .collect();

    for (node, layout) in surface.nodes.iter().zip(node_layouts.iter()) {
        let initialized_index = initialized_values.len();
        let scratch_index = scratch_types.len();
        let shared_index = shared_types.len();

        initialized_values.push(layout.initialized_const.into());
        scratch_types.push(layout.scratch_struct);
        shared_types.push(layout.shared_struct);
        pointer_types.push(layout.pointer_struct);
        let new_pointer_source = PointerSource::Aggregate(
            PointerSourceAggregateType::Struct,
            map_pointer_sources(
                layout.pointer_sources.iter().cloned(),
                |mut indices| {
                    indices.insert(0, initialized_index);
                    PointerSource::Initialized(indices)
                },
                |mut indices| {
                    indices.insert(0, scratch_index);
                    PointerSource::Scratch(indices)
                },
                |mut indices| {
                    indices.insert(0, shared_index);
                    PointerSource::Shared(indices)
                },
                |socket_index, sub_indices| {
                    let socket_group = node.sockets[socket_index].group_id;
                    let new_pointer_source = group_pointers[socket_group].clone();
                    append_path_to_pointer_source(new_pointer_source, &sub_indices)
                },
            ),
        );
        pointer_sources.push(new_pointer_source);
    }

    let initialized_val_refs: Vec<_> = initialized_values
        .iter()
        .map(|x| x as &BasicValue)
        .collect();
    let scratch_type_refs: Vec<_> = scratch_types.iter().map(|x| x as &BasicType).collect();
    let shared_type_refs: Vec<_> = shared_types.iter().map(|x| x as &BasicType).collect();
    let pointer_type_refs: Vec<_> = pointer_types.iter().map(|x| x as &BasicType).collect();

    SurfaceLayout {
        initialized_const: context.const_struct(&initialized_val_refs, false),
        scratch_struct: context.struct_type(&scratch_type_refs, false),
        shared_struct: context.struct_type(&shared_type_refs, false),
        pointer_struct: context.struct_type(&pointer_type_refs, false),
        node_layouts,
        pointer_sources,
        node_scratch_offset,
        node_initializer_offset,
    }
}

fn modify_pointer_source(
    source: PointerSource,
    initialized_modifier: &Fn(Vec<usize>) -> PointerSource,
    scratch_modifier: &Fn(Vec<usize>) -> PointerSource,
    shared_modifier: &Fn(Vec<usize>) -> PointerSource,
    socket_modifier: &Fn(usize, Vec<usize>) -> PointerSource,
) -> PointerSource {
    match source {
        PointerSource::Initialized(indices) => initialized_modifier(indices),
        PointerSource::Scratch(indices) => scratch_modifier(indices),
        PointerSource::Shared(indices) => shared_modifier(indices),
        PointerSource::Socket(socket_index, sub_indices) => {
            socket_modifier(socket_index, sub_indices)
        }
        PointerSource::Aggregate(aggregate_type, mut sources) => {
            for src in sources.iter_mut() {
                *src = modify_pointer_source(
                    src.clone(),
                    initialized_modifier,
                    scratch_modifier,
                    shared_modifier,
                    socket_modifier,
                )
            }
            PointerSource::Aggregate(aggregate_type, sources)
        }
    }
}

fn map_pointer_sources(
    sources: impl IntoIterator<Item = PointerSource>,
    initialized_modifier: impl Fn(Vec<usize>) -> PointerSource,
    scratch_modifier: impl Fn(Vec<usize>) -> PointerSource,
    shared_modifier: impl Fn(Vec<usize>) -> PointerSource,
    socket_modifier: impl Fn(usize, Vec<usize>) -> PointerSource,
) -> Vec<PointerSource> {
    sources
        .into_iter()
        .map(|pointer_source| {
            modify_pointer_source(
                pointer_source,
                &initialized_modifier,
                &scratch_modifier,
                &shared_modifier,
                &socket_modifier,
            )
        })
        .collect()
}

fn append_path_to_pointer_source(source: PointerSource, path: &[usize]) -> PointerSource {
    modify_pointer_source(
        source,
        &|mut indices| {
            indices.extend(path);
            PointerSource::Initialized(indices)
        },
        &|mut indices| {
            indices.extend(path);
            PointerSource::Scratch(indices)
        },
        &|mut indices| {
            indices.extend(path);
            PointerSource::Shared(indices)
        },
        &|socket_index, mut sub_indices| {
            sub_indices.extend(path);
            PointerSource::Socket(socket_index, sub_indices)
        },
    )
}

impl NodeLayout {
    pub fn scratch_index() -> usize {
        0
    }

    pub fn ui_index() -> usize {
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

    pub fn node_scratch_index(&self, node: usize) -> usize {
        self.node_scratch_offset + node
    }

    pub fn node_ptr_index(&self, node: usize) -> usize {
        node
    }
}
