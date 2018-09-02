use codegen::data_analyzer::{PointerSource, PointerSourceAggregateType};
use codegen::values::remap_type;
use codegen::{build_context_function, surface, util, BuilderContext, LifecycleFunc, ObjectCache};
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::BasicType;
use inkwell::values::{BasicValue, BasicValueEnum, GlobalValue, IntValue, PointerValue};
use inkwell::AddressSpace;
use mir::{Root, SurfaceRef};
use std::iter;

fn get_gep_indices(context: &Context, path: impl IntoIterator<Item = u64>) -> Vec<IntValue> {
    iter::once(0)
        .chain(path)
        .map(|index| context.i32_type().const_int(index as u64, false))
        .collect()
}

pub fn remap_pointer_source(
    context: &Context,
    src: &PointerSource,
    initialized: PointerValue,
    scratch: PointerValue,
    sockets: PointerValue,
) -> BasicValueEnum {
    match src {
        PointerSource::Initialized(path) => {
            let gep_indices = get_gep_indices(context, path.iter().map(|itm| *itm as u64));
            unsafe { initialized.const_in_bounds_gep(&gep_indices) }.into()
        }
        PointerSource::Scratch(path) => {
            let gep_indices = get_gep_indices(context, path.iter().map(|itm| *itm as u64));
            unsafe { scratch.const_in_bounds_gep(&gep_indices) }.into()
        }
        PointerSource::Socket(socket, path) => {
            let gep_indices = get_gep_indices(
                context,
                iter::once(*socket as u64).chain(path.iter().map(|itm| *itm as u64)),
            );
            unsafe { sockets.const_in_bounds_gep(&gep_indices) }.into()
        }
        PointerSource::Aggregate(agg_type, ref sub_sources) => {
            let sub_values: Vec<_> = sub_sources
                .iter()
                .map(|source| remap_pointer_source(context, source, initialized, scratch, sockets))
                .collect();

            match agg_type {
                PointerSourceAggregateType::Struct => {
                    let sub_value_refs: Vec<_> =
                        sub_values.iter().map(|val| val as &BasicValue).collect();
                    context.const_struct(&sub_value_refs, false).into()
                }
                PointerSourceAggregateType::Array => {
                    // we need to use the first item to figure out the array type
                    let array_val =
                        util::get_constant_array(&sub_values[0].get_type(), &sub_values);
                    array_val.into()
                }
            }
        }
    }
}

pub fn build_initialized_global(
    module: &Module,
    cache: &ObjectCache,
    surface: SurfaceRef,
    name: &str,
) -> GlobalValue {
    let layout = cache.surface_layout(surface).unwrap();
    let global = util::get_or_create_global(module, name, &layout.initialized_const.get_type());
    global.set_initializer(&layout.initialized_const);
    //global.set_section("maxim.init");
    global
}

pub fn build_scratch_global(
    module: &Module,
    cache: &ObjectCache,
    surface: SurfaceRef,
    name: &str,
) -> GlobalValue {
    let layout = cache.surface_layout(surface).unwrap();
    let global = util::get_or_create_global(module, name, &layout.scratch_struct);
    global.set_initializer(&layout.scratch_struct.const_null());
    //global.set_section("maxim.scratch");
    global
}

pub struct SocketsGlobal {
    pub sockets: GlobalValue,
    pub socket_ptrs: GlobalValue,
}

pub fn build_sockets_global(
    module: &Module,
    root: &Root,
    sockets_name: &str,
    pointers_name: &str,
) -> SocketsGlobal {
    let context = module.get_context();
    let struct_types: Vec<_> = root.sockets
        .iter()
        .map(|vartype| remap_type(&context, vartype))
        .collect();
    let sockets_type_refs: Vec<_> = struct_types.iter().map(|ty| ty as &BasicType).collect();
    let sockets_struct_type = context.struct_type(&sockets_type_refs, false);
    let sockets_global = util::get_or_create_global(module, sockets_name, &sockets_struct_type);
    sockets_global.set_initializer(&sockets_struct_type.const_null());
    //sockets_global.set_section("maxim.sockets");

    let void_ptr_ty = context.i8_type().ptr_type(AddressSpace::Generic);
    let array_itms: Vec<_> = (0..struct_types.len())
        .map(|index| unsafe {
            sockets_global
                .as_pointer_value()
                .const_in_bounds_gep(&[
                    context.i64_type().const_int(0, false),
                    context.i32_type().const_int(index as u64, false),
                ])
                .const_cast(&void_ptr_ty)
        })
        .collect();
    let pointers_arr = void_ptr_ty.const_array(&array_itms);
    let pointers_global =
        util::get_or_create_global(module, pointers_name, &pointers_arr.get_type());
    pointers_global.set_constant(true);
    pointers_global.set_initializer(&pointers_arr);
    //pointers_global.set_section("maxim.portals");

    SocketsGlobal {
        sockets: sockets_global,
        socket_ptrs: pointers_global,
    }
}

pub fn build_pointers_global(
    module: &Module,
    cache: &ObjectCache,
    surface: SurfaceRef,
    name: &str,
    initialized: PointerValue,
    scratch: PointerValue,
    sockets: PointerValue,
) -> GlobalValue {
    let layout = cache.surface_layout(surface).unwrap();
    let global = util::get_or_create_global(module, name, &layout.pointer_struct);
    let const_val = remap_pointer_source(
        &module.get_context(),
        &PointerSource::Aggregate(
            PointerSourceAggregateType::Struct,
            layout.pointer_sources.clone(),
        ),
        initialized,
        scratch,
        sockets,
    );
    global.set_constant(true);
    global.set_initializer(&const_val);
    //global.set_section("maxim.pointers");
    global
}

pub fn build_lifecycle_func(
    module: &Module,
    cache: &ObjectCache,
    surface: SurfaceRef,
    name: &str,
    lifecycle: LifecycleFunc,
    pointers: PointerValue,
) {
    let func = util::get_or_create_func(module, name, true, &|| {
        (
            Linkage::ExternalLinkage,
            module.get_context().void_type().fn_type(&[], false),
        )
    });
    build_context_function(module, func, cache.target(), &|ctx: BuilderContext| {
        surface::build_lifecycle_call(module, cache, ctx.b, surface, lifecycle, pointers);
        ctx.b.build_return(None);
    });
}

pub fn build_funcs(
    module: &Module,
    cache: &ObjectCache,
    surface: SurfaceRef,
    construct_name: &str,
    update_name: &str,
    destruct_name: &str,
    pointers: PointerValue,
) {
    build_lifecycle_func(
        module,
        cache,
        surface,
        construct_name,
        LifecycleFunc::Construct,
        pointers,
    );
    build_lifecycle_func(
        module,
        cache,
        surface,
        update_name,
        LifecycleFunc::Update,
        pointers,
    );
    build_lifecycle_func(
        module,
        cache,
        surface,
        destruct_name,
        LifecycleFunc::Destruct,
        pointers,
    );
}
