use codegen::data_analyzer::{PointerSource, PointerSourceAggregateType};
use codegen::{build_context_function, surface, util, BuilderContext, LifecycleFunc, ObjectCache};
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::values::{BasicValue, BasicValueEnum, GlobalValue, IntValue, PointerValue};
use inkwell::AddressSpace;
use mir::SurfaceRef;
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
    global.set_section("maxim.initialized_data");
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
    global.set_section("maxim.scratch_data");
    global
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
    println!("Expected type: {:#?}", layout.pointer_struct);
    println!("Actual type: {:#?}", const_val.get_type());
    println!("Layout: {:?}", layout.pointer_sources);
    global.set_initializer(&const_val);
    global.set_section("maxim.pointers_data");
    global
}

pub fn build_lifecycle_func(
    module: &Module,
    cache: &ObjectCache,
    surface: SurfaceRef,
    name: &str,
    lifecycle: LifecycleFunc,
    scratch: PointerValue,
    pointers: PointerValue,
) {
    let func = util::get_or_create_func(module, name, true, &|| {
        (
            Linkage::ExternalLinkage,
            module.get_context().void_type().fn_type(&[], false),
        )
    });
    build_context_function(module, func, cache.target(), &|mut ctx: BuilderContext| {
        surface::build_lifecycle_call(module, cache, ctx.b, surface, lifecycle, scratch, pointers);
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
    scratch: PointerValue,
    pointers: PointerValue,
) {
    build_lifecycle_func(
        module,
        cache,
        surface,
        construct_name,
        LifecycleFunc::Construct,
        scratch,
        pointers,
    );
    build_lifecycle_func(
        module,
        cache,
        surface,
        update_name,
        LifecycleFunc::Update,
        scratch,
        pointers,
    );
    build_lifecycle_func(
        module,
        cache,
        surface,
        destruct_name,
        LifecycleFunc::Destruct,
        scratch,
        pointers,
    );
}
