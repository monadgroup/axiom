use codegen::{
    block, build_context_function, data_analyzer, util, BuilderContext, LifecycleFunc,
    TargetProperties,
};
use inkwell::builder::Builder;
use inkwell::module::{Linkage, Module};
use inkwell::values::{FunctionValue, PointerValue};
use inkwell::AddressSpace;
use mir::{MIRContext, Node, NodeData, Surface};

fn get_lifecycle_func(
    module: &Module,
    mir: &MIRContext,
    surface: &Surface,
    target: &TargetProperties,
    lifecycle: LifecycleFunc,
) -> FunctionValue {
    let func_name = format!(
        "maxim.surface.{}.{}.{}",
        surface.id.id, surface.id.debug_name, lifecycle
    );
    util::get_or_create_func(module, &func_name, &|| {
        let context = module.get_context();
        let layout = data_analyzer::build_surface_layout(&context, mir, surface, target);
        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(
                &[
                    &layout.scratch_struct.ptr_type(AddressSpace::Generic),
                    &layout.pointer_struct.ptr_type(AddressSpace::Generic),
                ],
                false,
            ),
        )
    })
}

fn build_node_call(
    ctx: &mut BuilderContext,
    mir: &MIRContext,
    node: &Node,
    target: &TargetProperties,
    lifecycle: LifecycleFunc,
    scratch_ptr: PointerValue,
    pointers_ptr: PointerValue,
) {
    match &node.data {
        NodeData::Custom(block_id) => {
            let block = mir.block(&block_id).unwrap();
            let data_ptr = unsafe { ctx.b.build_struct_gep(&scratch_ptr, 0, "") };
            let ui_ptr = if target.include_ui {
                Some(unsafe { ctx.b.build_struct_gep(&scratch_ptr, 1, "") })
            } else {
                None
            };

            block::build_lifecycle_call(
                ctx.module,
                ctx.b,
                block,
                target,
                lifecycle,
                data_ptr,
                pointers_ptr,
                ui_ptr,
            );
        }
        NodeData::Group(surface_id) => {
            let surface = mir.surface(&surface_id).unwrap();
            build_lifecycle_call(
                ctx.module,
                ctx.b,
                mir,
                surface,
                target,
                lifecycle,
                scratch_ptr,
                pointers_ptr,
            );
        }
        NodeData::ExtractGroup { .. } => unimplemented!(),
    }
}

pub fn build_lifecycle_func(
    module: &Module,
    mir: &MIRContext,
    surface: &Surface,
    target: &TargetProperties,
    lifecycle: LifecycleFunc,
) {
    let func = get_lifecycle_func(module, mir, surface, target, lifecycle);
    build_context_function(module, func, target, &|mut ctx: BuilderContext| {
        let layout = data_analyzer::build_surface_layout(ctx.context, mir, surface, target);
        let scratch_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();
        let pointers_ptr = ctx.func.get_nth_param(1).unwrap().into_pointer_value();

        for (node_index, node) in surface.nodes.iter().enumerate() {
            let layout_ptr_index = layout.node_ptr_index(node_index);
            let node_scratch_ptr = unsafe {
                ctx.b.build_struct_gep(
                    &scratch_ptr,
                    layout.node_scratch_index(node_index) as u32,
                    "",
                )
            };
            let node_pointers_ptr = unsafe {
                ctx.b
                    .build_struct_gep(&pointers_ptr, layout_ptr_index as u32, "")
            };

            build_node_call(
                &mut ctx,
                mir,
                node,
                target,
                lifecycle,
                node_scratch_ptr,
                node_pointers_ptr,
            );
        }

        ctx.b.build_return(None);
    })
}

pub fn build_funcs(
    module: &Module,
    mir: &MIRContext,
    surface: &Surface,
    target: &TargetProperties,
) {
    build_lifecycle_func(module, mir, surface, target, LifecycleFunc::Construct);
    build_lifecycle_func(module, mir, surface, target, LifecycleFunc::Update);
    build_lifecycle_func(module, mir, surface, target, LifecycleFunc::Destruct);
}

pub fn build_lifecycle_call(
    module: &Module,
    builder: &mut Builder,
    mir: &MIRContext,
    surface: &Surface,
    target: &TargetProperties,
    lifecycle: LifecycleFunc,
    scratch_ptr: PointerValue,
    pointer_ptr: PointerValue,
) {
    let func = get_lifecycle_func(module, mir, surface, target, lifecycle);
    builder.build_call(&func, &[&scratch_ptr, &pointer_ptr], "", false);
}
