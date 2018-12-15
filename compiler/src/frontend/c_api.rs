use super::{exporter, value_reader, Runtime, Transaction};
use crate::{ast, codegen, mir, parser, pass, CompileError};
use inkwell::{orc, targets};
use std;
use std::os::raw::c_void;

#[no_mangle]
pub extern "C" fn maxim_initialize() {
    targets::Target::initialize_native(&targets::InitializationConfig::default()).unwrap();
    orc::Orc::link_in_jit();
}

#[no_mangle]
pub unsafe extern "C" fn maxim_destroy_string(string: *mut std::os::raw::c_char) {
    std::ffi::CString::from_raw(string);
    // string will be dropped here
}

#[no_mangle]
pub unsafe extern "C" fn maxim_create_runtime(include_ui: bool, min_size: bool) -> *mut Runtime {
    let target =
        codegen::TargetProperties::new(include_ui, min_size, targets::TargetMachine::select());
    Box::into_raw(Box::new(Runtime::new(target)))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_destroy_runtime(runtime: *mut Runtime) {
    Box::from_raw(runtime);
    // box will be dropped here
}

#[no_mangle]
pub unsafe extern "C" fn maxim_allocate_id(runtime: *mut Runtime) -> u64 {
    use crate::mir::IdAllocator;
    (*runtime).alloc_id()
}

#[no_mangle]
pub unsafe extern "C" fn maxim_export_transaction(min_size: bool, transaction: *mut Transaction) {
    let target = codegen::TargetProperties::new(false, min_size, targets::TargetMachine::select());
    let owned_transaction = Box::from_raw(transaction);
    exporter::build_module_from_transaction(&target, *owned_transaction);
}

#[no_mangle]
pub unsafe extern "C" fn maxim_run_update(runtime: *const Runtime) {
    (*runtime).run_update();
}

#[no_mangle]
pub unsafe extern "C" fn maxim_set_bpm(runtime: *mut Runtime, bpm: f64) {
    (*runtime).set_bpm(bpm);
}

#[no_mangle]
pub unsafe extern "C" fn maxim_get_bpm(runtime: *const Runtime) -> f64 {
    (*runtime).get_bpm()
}

#[no_mangle]
pub unsafe extern "C" fn maxim_set_sample_rate(runtime: *mut Runtime, sample_rate: f64) {
    (*runtime).set_sample_rate(sample_rate);
}

#[no_mangle]
pub unsafe extern "C" fn maxim_get_sample_rate(runtime: *const Runtime) -> f64 {
    (*runtime).get_sample_rate()
}

#[no_mangle]
pub unsafe extern "C" fn maxim_get_profile_times_ptr(runtime: *const Runtime) -> *mut u64 {
    (*runtime).get_profile_times_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn maxim_commit(runtime: *mut Runtime, transaction: *mut Transaction) {
    let owned_transaction = Box::from_raw(transaction);
    (*runtime).commit(*owned_transaction)
}

#[no_mangle]
pub unsafe extern "C" fn maxim_is_node_extracted(
    runtime: *const Runtime,
    surface: u64,
    node: usize,
) -> bool {
    (*runtime).is_node_extracted(surface, node)
}

#[no_mangle]
pub unsafe extern "C" fn maxim_convert_num(
    runtime: *const Runtime,
    result: *mut c_void,
    target_form: i8,
    num: *const c_void,
) {
    (*runtime).convert_num(result, target_form, num)
}

#[no_mangle]
pub unsafe extern "C" fn maxim_get_portal_ptr(
    runtime: *const Runtime,
    portal: usize,
) -> *mut c_void {
    (*runtime).get_portal_ptr(portal)
}

#[no_mangle]
pub unsafe extern "C" fn maxim_get_root_ptr(runtime: *const Runtime) -> *mut c_void {
    (*runtime).get_root_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn maxim_get_node_ptr(
    runtime: *const Runtime,
    surface: u64,
    surface_ptr: *mut c_void,
    node: usize,
) -> *mut c_void {
    value_reader::get_node_ptr(&*runtime, surface, surface_ptr, node)
}

#[no_mangle]
pub unsafe extern "C" fn maxim_get_extracted_bitmask_ptr(
    runtime: *const Runtime,
    surface: u64,
    surface_ptr: *mut c_void,
    node: usize,
) -> *const u32 {
    value_reader::get_node_active_bitmap_ptr(&*runtime, surface, surface_ptr, node)
}

#[no_mangle]
pub extern "C" fn maxim_get_surface_ptr(node_ptr: *mut c_void) -> *mut c_void {
    value_reader::get_surface_ptr(node_ptr)
}

#[no_mangle]
pub extern "C" fn maxim_get_block_ptr(node_ptr: *mut c_void) -> *mut c_void {
    value_reader::get_block_ptr(node_ptr)
}

#[no_mangle]
pub unsafe extern "C" fn maxim_get_control_ptrs(
    runtime: *const Runtime,
    block: u64,
    block_ptr: *mut c_void,
    control: usize,
) -> value_reader::ControlPointers {
    value_reader::get_control_ptrs(&*runtime, block, block_ptr, control)
}

#[no_mangle]
pub extern "C" fn maxim_create_transaction() -> *mut Transaction {
    Box::into_raw(Box::new(Transaction::new(None, Vec::new(), Vec::new())))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_clone_transaction(val: *const Transaction) -> *mut Transaction {
    Box::into_raw(Box::new((*val).clone()))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_destroy_transaction(val: *mut Transaction) {
    Box::from_raw(val);
    // box will be dropped here
}

#[no_mangle]
pub unsafe extern "C" fn maxim_print_transaction_to_stdout(val: *const Transaction) {
    println!("{:#?}", *val);
}

#[no_mangle]
pub extern "C" fn maxim_vartype_num() -> *mut mir::VarType {
    Box::into_raw(Box::new(mir::VarType::Num))
}

#[no_mangle]
pub extern "C" fn maxim_vartype_midi() -> *mut mir::VarType {
    Box::into_raw(Box::new(mir::VarType::Midi))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_vartype_tuple(
    subtypes: *const *mut mir::VarType,
    subtype_count: usize,
) -> *mut mir::VarType {
    let subtypes_vec: Vec<_> = (0..subtype_count)
        .map(|index| {
            let boxed = Box::from_raw(*subtypes.add(index));
            *boxed
        })
        .collect();
    Box::into_raw(Box::new(mir::VarType::Tuple(subtypes_vec)))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_vartype_array(subtype: *mut mir::VarType) -> *mut mir::VarType {
    Box::into_raw(Box::new(mir::VarType::Array(Box::from_raw(subtype))))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_vartype_of_control(control_type: u8) -> *mut mir::VarType {
    Box::into_raw(Box::new(mir::VarType::of_control_value(
        std::mem::transmute(control_type),
    )))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_vartype_clone(base: *const mir::VarType) -> *mut mir::VarType {
    Box::into_raw(Box::new((*base).clone()))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_destroy_vartype(val: *mut mir::VarType) {
    Box::from_raw(val);
    // box will be dropped here
}

#[no_mangle]
pub unsafe extern "C" fn maxim_constant_num(
    left: f64,
    right: f64,
    form: u8,
) -> *mut mir::ConstantValue {
    Box::into_raw(Box::new(mir::ConstantValue::new_num(
        left,
        right,
        std::mem::transmute(form),
    )))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_constant_tuple(
    items: *const *mut mir::ConstantValue,
    item_count: usize,
) -> *mut mir::ConstantValue {
    let items_vec: Vec<_> = (0..item_count)
        .map(|index| {
            let boxed = Box::from_raw(*items.add(index));
            *boxed
        })
        .collect();
    Box::into_raw(Box::new(mir::ConstantValue::Tuple(mir::ConstantTuple {
        items: items_vec,
    })))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_constant_clone(
    base: *const mir::ConstantValue,
) -> *mut mir::ConstantValue {
    Box::into_raw(Box::new((*base).clone()))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_destroy_constant(val: *mut mir::ConstantValue) {
    Box::from_raw(val);
    // box will be dropped here
}

#[no_mangle]
pub extern "C" fn maxim_valuegroupsource_none() -> *mut mir::ValueGroupSource {
    Box::into_raw(Box::new(mir::ValueGroupSource::None))
}

#[no_mangle]
pub extern "C" fn maxim_valuegroupsource_socket(index: usize) -> *mut mir::ValueGroupSource {
    Box::into_raw(Box::new(mir::ValueGroupSource::Socket(index)))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_valuegroupsource_default(
    value: *mut mir::ConstantValue,
) -> *mut mir::ValueGroupSource {
    let const_val = Box::from_raw(value);
    Box::into_raw(Box::new(mir::ValueGroupSource::Default(*const_val)))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_valuegroupsource_clone(
    base: *const mir::ValueGroupSource,
) -> *mut mir::ValueGroupSource {
    Box::into_raw(Box::new((*base).clone()))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_destroy_valuegroupsource(val: *mut mir::ValueGroupSource) {
    Box::from_raw(val);
    // box will be dropped here
}

#[no_mangle]
pub unsafe extern "C" fn maxim_build_root(transaction: *mut Transaction) -> *mut mir::Root {
    if let Some(ref mut root) = (*transaction).root {
        root
    } else {
        (*transaction).root = Some(mir::Root::new(Vec::new()));
        (*transaction).root.as_mut().unwrap()
    }
}

#[no_mangle]
pub unsafe extern "C" fn maxim_build_root_socket(root: *mut mir::Root, vartype: *mut mir::VarType) {
    let owned_vartype = Box::from_raw(vartype);
    (*root).sockets.push(*owned_vartype);
}

#[no_mangle]
pub unsafe extern "C" fn maxim_build_surface(
    transaction: *mut Transaction,
    id: u64,
    c_name: *const std::os::raw::c_char,
) -> *mut mir::Surface {
    let name = std::ffi::CStr::from_ptr(c_name)
        .to_str()
        .unwrap()
        .to_string();
    let new_surface = mir::Surface::new(
        mir::SurfaceId::new_with_id(name, id),
        Vec::new(),
        Vec::new(),
    );
    (*transaction).add_surface(new_surface);
    (*transaction).surfaces.get_mut(&id).unwrap()
}

#[no_mangle]
pub unsafe extern "C" fn maxim_build_value_group(
    surface: *mut mir::Surface,
    vartype: *mut mir::VarType,
    source: *mut mir::ValueGroupSource,
) {
    let owned_vartype = Box::from_raw(vartype);
    let owned_source = Box::from_raw(source);

    (*surface)
        .groups
        .push(mir::ValueGroup::new(*owned_vartype, *owned_source));
}

#[no_mangle]
pub unsafe extern "C" fn maxim_build_custom_node(
    surface: *mut mir::Surface,
    block_id: u64,
) -> *mut mir::Node {
    let new_node = mir::Node::new(Vec::new(), mir::NodeData::Custom(block_id));
    (*surface).nodes.push(new_node);
    &mut (*surface).nodes[(*surface).nodes.len() - 1]
}

#[no_mangle]
pub unsafe extern "C" fn maxim_build_group_node(
    surface: *mut mir::Surface,
    surface_id: u64,
) -> *mut mir::Node {
    let new_node = mir::Node::new(Vec::new(), mir::NodeData::Group(surface_id));
    (*surface).nodes.push(new_node);
    &mut (*surface).nodes[(*surface).nodes.len() - 1]
}

#[no_mangle]
pub unsafe extern "C" fn maxim_build_value_socket(
    node: *mut mir::Node,
    group_id: usize,
    value_written: bool,
    value_read: bool,
    is_extractor: bool,
) {
    (*node).sockets.push(mir::ValueSocket::new(
        group_id,
        value_written,
        value_read,
        is_extractor,
    ));
}

#[no_mangle]
pub unsafe extern "C" fn maxim_build_block(transaction: *mut Transaction, block: *mut mir::Block) {
    let owned_block = Box::from_raw(block);
    (*transaction).add_block(*owned_block);
}

#[no_mangle]
pub unsafe extern "C" fn maxim_compile_block(
    id: u64,
    c_name: *const std::os::raw::c_char,
    c_code: *const std::os::raw::c_char,
    success_block_out: *mut *mut mir::Block,
    fail_error_out: *mut *mut CompileError,
) -> bool {
    let name = std::ffi::CStr::from_ptr(c_name)
        .to_str()
        .unwrap()
        .to_string();
    let code = std::ffi::CStr::from_ptr(c_code).to_str().unwrap();

    let mut stream = parser::get_token_stream(code);
    match parser::Parser::parse(&mut stream)
        .and_then(|ast| pass::lower_ast(mir::BlockId::new_with_id(name, id), &ast))
    {
        Ok(block) => {
            *success_block_out = Box::into_raw(Box::new(block));
            true
        }
        Err(err) => {
            *fail_error_out = Box::into_raw(Box::new(err));
            false
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn maxim_destroy_block(val: *mut mir::Block) {
    Box::from_raw(val);
    // box will be dropped here
}

#[no_mangle]
pub unsafe extern "C" fn maxim_block_clone(block: *const mir::Block) -> *mut mir::Block {
    Box::into_raw(Box::new((*block).clone()))
}

#[no_mangle]
pub unsafe extern "C" fn maxim_error_get_description(
    error: *const CompileError,
) -> *mut std::os::raw::c_char {
    std::ffi::CString::new((*error).to_string())
        .unwrap()
        .into_raw()
}

#[no_mangle]
pub unsafe extern "C" fn maxim_error_get_range(error: *const CompileError) -> ast::SourceRange {
    (*error).range()
}

#[no_mangle]
pub unsafe extern "C" fn maxim_destroy_error(error: *mut CompileError) {
    Box::from_raw(error);
    // box will be dropped here
}

#[no_mangle]
pub unsafe extern "C" fn maxim_block_get_control_count(block: *const mir::Block) -> usize {
    (*block).controls.len()
}

#[no_mangle]
pub unsafe extern "C" fn maxim_block_get_control(
    block: *mut mir::Block,
    index: usize,
) -> *mut mir::block::Control {
    &mut (*block).controls[index]
}

#[no_mangle]
pub unsafe extern "C" fn maxim_control_get_name(
    control: *const mir::block::Control,
) -> *mut std::os::raw::c_char {
    std::ffi::CString::new((*control).name.clone())
        .unwrap()
        .into_raw()
}

#[no_mangle]
pub unsafe extern "C" fn maxim_control_get_type(control: *const mir::block::Control) -> u8 {
    (*control).control_type as u8
}

#[no_mangle]
pub unsafe extern "C" fn maxim_control_get_written(control: *const mir::block::Control) -> bool {
    (*control).value_written
}

#[no_mangle]
pub unsafe extern "C" fn maxim_control_get_read(control: *const mir::block::Control) -> bool {
    (*control).value_read
}

#[no_mangle]
pub unsafe extern "C" fn maxim_get_function_table_size() -> usize {
    mir::FUNCTION_TABLE.len()
}

#[no_mangle]
pub unsafe extern "C" fn maxim_get_function_table_entry(index: usize) -> *mut std::os::raw::c_char {
    std::ffi::CString::new(mir::FUNCTION_TABLE[index])
        .unwrap()
        .into_raw()
}
