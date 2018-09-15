use codegen::data_analyzer::SurfaceLayout;
use codegen::ObjectCache;
use codegen::TargetProperties;
use mir::{BlockRef, InternalNodeRef, NodeData, SurfaceRef};
use std::os::raw::c_void;
use std::ptr::null_mut;

pub type SurfacePtr = *mut c_void;
pub type NodePtr = *mut c_void;
pub type BlockPtr = *mut c_void;
pub type ControlValuePtr = *mut c_void;
pub type ControlDataPtr = *mut c_void;
pub type ControlSharedPtr = *mut c_void;
pub type ControlUiPtr = *mut c_void;

#[repr(C)]
pub struct ControlPointers {
    pub value: ControlValuePtr,
    pub data: ControlDataPtr,
    pub shared: ControlSharedPtr,
    pub ui: ControlUiPtr,
}

fn get_internal_node_ptr(
    target: &TargetProperties,
    layout: &SurfaceLayout,
    ptr: SurfacePtr,
    node: usize,
) -> NodePtr {
    let ptr_offset = layout.node_ptr_index(node);
    let byte_offset = target
        .machine
        .get_data()
        .offset_of_element(&layout.pointer_struct, ptr_offset as u32)
        .unwrap();
    unsafe { ptr.offset(byte_offset as isize) }
}

pub fn get_node_ptr(
    cache: &ObjectCache,
    surface: SurfaceRef,
    ptr: SurfacePtr,
    node: usize,
) -> NodePtr {
    let surface_mir = cache.surface_mir(surface).unwrap();
    let surface_layout = cache.surface_layout(surface).unwrap();
    match surface_mir.source_map.map_to_internal(node) {
        InternalNodeRef::Direct(node) => {
            get_internal_node_ptr(cache.target(), surface_layout, ptr, node)
        }
        InternalNodeRef::Surface(surface_node, node) => {
            let subsurface_ptr = get_surface_ptr(get_internal_node_ptr(
                cache.target(),
                surface_layout,
                ptr,
                surface_node,
            ));
            let subsurface_ref = match surface_mir.nodes[surface_node].data {
                NodeData::Group(subsurface_ref) => subsurface_ref,
                NodeData::ExtractGroup { surface, .. } => surface,
                _ => panic!("Sourcemap Surface reference points to a non-surface node"),
            };
            get_node_ptr(cache, subsurface_ref, subsurface_ptr, node)
        }
    }
}

pub fn get_surface_ptr(ptr: NodePtr) -> SurfacePtr {
    ptr
}

pub fn get_block_ptr(ptr: NodePtr) -> BlockPtr {
    ptr
}

pub fn get_control_ptrs(
    cache: &ObjectCache,
    block: BlockRef,
    ptr: BlockPtr,
    control: usize,
) -> ControlPointers {
    let block_layout = cache.block_layout(block).unwrap();
    let ptr_offset = block_layout.control_index(control);
    let byte_offset = cache
        .target()
        .machine
        .get_data()
        .offset_of_element(&block_layout.pointer_struct, ptr_offset as u32)
        .unwrap();
    let base_ptr = unsafe { ptr.offset(byte_offset as isize) } as *mut *mut c_void;
    ControlPointers {
        value: unsafe { *base_ptr },
        data: unsafe { *base_ptr.offset(1) },
        shared: unsafe { *base_ptr.offset(2) },
        ui: if cache.target().include_ui {
            unsafe { *base_ptr.offset(3) }
        } else {
            null_mut()
        },
    }
}
