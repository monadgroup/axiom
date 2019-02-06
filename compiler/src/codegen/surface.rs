use crate::codegen::{
    block, build_context_function, util, values, BuilderContext, LifecycleFunc, ObjectCache,
};
use crate::mir::{Node, NodeData, Surface, SurfaceRef};
use inkwell::builder::Builder;
use inkwell::module::{Linkage, Module};
use inkwell::values::{FunctionValue, PointerValue};
use inkwell::{AddressSpace, IntPredicate};

fn get_lifecycle_func(
    module: &Module,
    cache: &ObjectCache,
    surface: SurfaceRef,
    lifecycle: LifecycleFunc,
) -> FunctionValue {
    let func_name = format!("maxim.surface.{}.{}", surface, lifecycle);
    util::get_or_create_func(module, &func_name, true, &|| {
        let context = module.get_context();
        let layout = cache.surface_layout(surface).unwrap();
        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(
                &[&layout.pointer_struct.ptr_type(AddressSpace::Generic)],
                false,
            ),
        )
    })
}

fn build_node_call(
    ctx: &mut BuilderContext,
    cache: &ObjectCache,
    node: &Node,
    lifecycle: LifecycleFunc,
    pointers_ptr: PointerValue,
) {
    match &node.data {
        NodeData::Dummy => {}
        NodeData::Custom(block_id) => {
            block::build_lifecycle_call(
                ctx.module,
                cache,
                ctx.b,
                *block_id,
                lifecycle,
                pointers_ptr,
            );
        }
        NodeData::Group(surface_id) => {
            build_lifecycle_call(
                ctx.module,
                cache,
                ctx.b,
                *surface_id,
                lifecycle,
                pointers_ptr,
            );
        }
        NodeData::ExtractGroup {
            surface: surface_id,
            source_sockets,
            dest_sockets,
        } => {
            let voice_pointers = unsafe { ctx.b.build_struct_gep(&pointers_ptr, 0, "voices.ptr") };
            let source_socket_pointers =
                unsafe { ctx.b.build_struct_gep(&pointers_ptr, 1, "sources.ptr") };
            let dest_socket_pointers =
                unsafe { ctx.b.build_struct_gep(&pointers_ptr, 2, "dests.ptr") };
            let bitmap_pointer =
                unsafe { ctx.b.build_struct_gep(&pointers_ptr, 3, "bitmap.ptr.ptr") };

            // if this is the update lifecycle function and there are source groups, generate a
            // bitmap of which indices are valid
            let valid_bitmap = if lifecycle == LifecycleFunc::Update && !source_sockets.is_empty() {
                let first_array = values::ArrayValue::new(
                    ctx.b
                        .build_load(
                            &unsafe { ctx.b.build_struct_gep(&source_socket_pointers, 0, "") },
                            "",
                        )
                        .into_pointer_value(),
                );
                let first_bitmap = first_array.get_bitmap(ctx.b);

                let active_bitmap =
                    (1..source_sockets.len()).fold(first_bitmap, |acc, socket_index| {
                        let nth_array = values::ArrayValue::new(
                            ctx.b
                                .build_load(
                                    &unsafe {
                                        ctx.b.build_struct_gep(
                                            &source_socket_pointers,
                                            socket_index as u32,
                                            "",
                                        )
                                    },
                                    "",
                                )
                                .into_pointer_value(),
                        );
                        let nth_bitmap = nth_array.get_bitmap(ctx.b);
                        ctx.b.build_and(acc, nth_bitmap, "")
                    });
                ctx.b.build_store(
                    &ctx.b
                        .build_load(&bitmap_pointer, "bitmap.ptr")
                        .into_pointer_value(),
                    &active_bitmap,
                );

                Some(active_bitmap)
            } else {
                None
            };

            // build a for loop to iterate over each instance
            let index_ptr = ctx
                .allocb
                .build_alloca(&ctx.context.i8_type(), "voiceindex.ptr");
            ctx.b
                .build_store(&index_ptr, &ctx.context.i8_type().const_int(0, false));

            let check_block = ctx.context.append_basic_block(&ctx.func, "voice.check");
            let check_active_block = ctx
                .context
                .append_basic_block(&ctx.func, "voice.checkactive");
            let run_block = ctx.context.append_basic_block(&ctx.func, "voice.run");
            let end_block = ctx.context.append_basic_block(&ctx.func, "voice.end");

            ctx.b.build_unconditional_branch(&check_block);
            ctx.b.position_at_end(&check_block);

            let current_index = ctx.b.build_load(&index_ptr, "voiceindex").into_int_value();
            let iter_limit = ctx
                .context
                .i8_type()
                .const_int(u64::from(values::ARRAY_CAPACITY), false);
            let can_continue_loop = ctx.b.build_int_compare(
                IntPredicate::ULT,
                current_index,
                iter_limit,
                "cancontinue",
            );

            ctx.b
                .build_conditional_branch(&can_continue_loop, &check_active_block, &end_block);
            ctx.b.position_at_end(&check_active_block);

            // increment the stored value
            let next_index = ctx.b.build_int_nuw_add(
                current_index,
                ctx.context.i8_type().const_int(1, false),
                "nextindex",
            );
            ctx.b.build_store(&index_ptr, &next_index);

            let index_32 = ctx
                .b
                .build_int_z_extend(current_index, ctx.context.i32_type(), "");
            if let Some(active_bitmap) = valid_bitmap {
                // check if this iteration is active according to the bitmap
                let active_bit = util::get_bit(ctx.b, active_bitmap, index_32);
                ctx.b
                    .build_conditional_branch(&active_bit, &run_block, &check_block);
            } else {
                ctx.b.build_unconditional_branch(&run_block);
            }

            ctx.b.position_at_end(&run_block);

            let const_zero = ctx.context.i32_type().const_int(0, false);
            let voice_pointers_ptr = unsafe {
                ctx.b
                    .build_in_bounds_gep(&voice_pointers, &[const_zero, index_32], "pointersptr")
            };

            build_lifecycle_call(
                ctx.module,
                cache,
                ctx.b,
                *surface_id,
                lifecycle,
                voice_pointers_ptr,
            );

            ctx.b.build_unconditional_branch(&check_block);
            ctx.b.position_at_end(&end_block);

            // set the bitmaps of all output arrays to be this input
            if lifecycle == LifecycleFunc::Update {
                let active_bitmap = if let Some(active_bitmap) = valid_bitmap {
                    active_bitmap
                } else {
                    ctx.b
                        .build_not(&ctx.context.i32_type().const_int(0, false), "")
                };

                for dest_socket_index in 0..dest_sockets.len() {
                    let dest_array = values::ArrayValue::new(
                        ctx.b
                            .build_load(
                                &unsafe {
                                    ctx.b.build_struct_gep(
                                        &dest_socket_pointers,
                                        dest_socket_index as u32,
                                        "",
                                    )
                                },
                                "",
                            )
                            .into_pointer_value(),
                    );
                    dest_array.set_bitmap(ctx.b, active_bitmap);
                }
            }
        }
    }
}

pub fn build_lifecycle_func(
    module: &Module,
    cache: &ObjectCache,
    surface: &Surface,
    lifecycle: LifecycleFunc,
) {
    let func = get_lifecycle_func(module, cache, surface.id.id, lifecycle);
    build_context_function(module, func, cache.target(), &|mut ctx: BuilderContext| {
        let layout = cache.surface_layout(surface.id.id).unwrap();
        let pointers_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();

        for (node_index, node) in surface.nodes.iter().enumerate() {
            let layout_ptr_index = layout.node_ptr_index(node_index);
            let node_pointers_ptr = unsafe {
                ctx.b
                    .build_struct_gep(&pointers_ptr, layout_ptr_index as u32, "")
            };

            build_node_call(&mut ctx, cache, node, lifecycle, node_pointers_ptr);
        }

        ctx.b.build_return(None);
    })
}

pub fn build_funcs(module: &Module, cache: &ObjectCache, surface: &Surface) {
    build_lifecycle_func(module, cache, surface, LifecycleFunc::Construct);
    build_lifecycle_func(module, cache, surface, LifecycleFunc::Update);
    build_lifecycle_func(module, cache, surface, LifecycleFunc::Destruct);
}

pub fn build_lifecycle_call(
    module: &Module,
    cache: &ObjectCache,
    builder: &mut Builder,
    surface: SurfaceRef,
    lifecycle: LifecycleFunc,
    pointer_ptr: PointerValue,
) {
    let func = get_lifecycle_func(module, cache, surface, lifecycle);
    builder.build_call(&func, &[&pointer_ptr], "", true);
}
