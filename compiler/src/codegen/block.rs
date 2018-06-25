use ast::ControlType;
use codegen::{
    build_context_function, controls, data_analyzer, functions, statements, util, BlockContext,
    BuilderContext, LifecycleFunc, TargetProperties,
};
use inkwell::builder::Builder;
use inkwell::module::{Linkage, Module};
use inkwell::values::{FunctionValue, PointerValue};
use inkwell::AddressSpace;
use mir::Block;

fn get_lifecycle_func(
    module: &Module,
    block: &Block,
    target: &TargetProperties,
    lifecycle: LifecycleFunc,
) -> FunctionValue {
    let func_name = format!(
        "maxim.block.{}.{}.{}",
        block.id.id, block.id.debug_name, lifecycle
    );
    util::get_or_create_func(module, &func_name, &|| {
        let context = module.get_context();
        let layout = data_analyzer::build_block_layout(&context, block, target);

        let func_type = if let Some(ui_struct) = layout.ui_struct {
            context.void_type().fn_type(
                &[
                    &layout.scratch_struct.ptr_type(AddressSpace::Generic),
                    &layout.groups_struct.ptr_type(AddressSpace::Generic),
                    &ui_struct.ptr_type(AddressSpace::Generic),
                ],
                false,
            )
        } else {
            context.void_type().fn_type(
                &[
                    &layout.scratch_struct.ptr_type(AddressSpace::Generic),
                    &layout.groups_struct.ptr_type(AddressSpace::Generic),
                ],
                false,
            )
        };

        (Linkage::ExternalLinkage, func_type)
    })
}

// Construct and destruct lifecycle functions call the respective lifecycle functions on
// the controls and functions.
// The update lifecycle function calls each controls update function, and then the code from
// the block (which we build here too).
// Note: if set in the target properties, we also need to call the UI lifecycle functions on
// controls. These are always constructed and updated after the regular functions, but
// destructed before.
fn build_lifecycle_func(
    module: &Module,
    block: &Block,
    target: &TargetProperties,
    lifecycle: LifecycleFunc,
    cb: &Fn(&mut BlockContext),
) {
    let func = get_lifecycle_func(module, block, target, lifecycle);
    build_context_function(module, func, &|ctx: BuilderContext| {
        let layout = data_analyzer::build_block_layout(&module.get_context(), block, target);
        let data_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();
        let group_ptr = ctx.func.get_nth_param(1).unwrap().into_pointer_value();
        let ui_ptr = if let Some(param) = ctx.func.get_nth_param(2) {
            Some(param.into_pointer_value())
        } else {
            None
        };
        let mut ctx = BlockContext::new(ctx, layout, data_ptr, group_ptr, ui_ptr);
        cb(&mut ctx);
        ctx.ctx.b.build_return(None);
    });
}

pub fn build_construct_func(module: &Module, block: &Block, target: &TargetProperties) {
    build_lifecycle_func(
        module,
        block,
        target,
        LifecycleFunc::Construct,
        &|block_ctx: &mut BlockContext| {
            for (control_index, control) in block.controls.iter().enumerate() {
                let layout_index = block_ctx.layout.control_index(control_index);
                let data_ptr = block_ctx.get_data_entry(layout_index);
                let group_ptr = block_ctx.get_group_entry(layout_index);
                controls::build_lifecycle_call(
                    module,
                    &mut block_ctx.ctx.b,
                    control.control_type,
                    LifecycleFunc::Construct,
                    group_ptr,
                    data_ptr,
                );

                if let Some(ui_ptr) = block_ctx.get_ui_entry(layout_index) {
                    controls::build_ui_lifecycle_call(
                        module,
                        &mut block_ctx.ctx.b,
                        control.control_type,
                        LifecycleFunc::Construct,
                        group_ptr,
                        data_ptr,
                        ui_ptr,
                    );
                }
            }

            for (func_index, function) in block_ctx.layout.functions.iter().enumerate() {
                let layout_index = block_ctx.layout.function_index(func_index);
                let data_ptr = block_ctx.get_data_entry(layout_index);
                functions::build_lifecycle_call(
                    module,
                    &mut block_ctx.ctx.b,
                    function,
                    functions::FunctionLifecycleFunc::Construct,
                    data_ptr,
                );
            }
        },
    )
}

pub fn build_update_func(module: &Module, block: &Block, target: &TargetProperties) {
    build_lifecycle_func(
        module,
        block,
        target,
        LifecycleFunc::Update,
        &|block_ctx: &mut BlockContext| {
            for (control_index, control) in block.controls.iter().enumerate() {
                let layout_index = block_ctx.layout.control_index(control_index);
                let data_ptr = block_ctx.get_data_entry(layout_index);
                let group_ptr = block_ctx.get_group_entry(layout_index);
                controls::build_lifecycle_call(
                    module,
                    &mut block_ctx.ctx.b,
                    control.control_type,
                    LifecycleFunc::Update,
                    group_ptr,
                    data_ptr,
                );

                if let Some(ui_ptr) = block_ctx.get_ui_entry(layout_index) {
                    controls::build_ui_lifecycle_call(
                        module,
                        &mut block_ctx.ctx.b,
                        control.control_type,
                        LifecycleFunc::Update,
                        group_ptr,
                        data_ptr,
                        ui_ptr,
                    );
                }
            }

            for (statement_index, statement) in block.statements.iter().enumerate() {
                let statement_result =
                    statements::gen_statement(statement_index, statement, block_ctx);
                block_ctx.push_statement(statement_result);
            }
        },
    )
}

pub fn build_destruct_func(module: &Module, block: &Block, target: &TargetProperties) {
    build_lifecycle_func(
        module,
        block,
        target,
        LifecycleFunc::Destruct,
        &|block_ctx: &mut BlockContext| {
            for (control_index, control) in block.controls.iter().enumerate() {
                let layout_index = block_ctx.layout.control_index(control_index);
                let data_ptr = block_ctx.get_data_entry(layout_index);
                let group_ptr = block_ctx.get_group_entry(layout_index);
                if let Some(ui_ptr) = block_ctx.get_ui_entry(layout_index) {
                    controls::build_ui_lifecycle_call(
                        module,
                        &mut block_ctx.ctx.b,
                        control.control_type,
                        LifecycleFunc::Destruct,
                        group_ptr,
                        data_ptr,
                        ui_ptr,
                    );
                }

                controls::build_lifecycle_call(
                    module,
                    &mut block_ctx.ctx.b,
                    control.control_type,
                    LifecycleFunc::Destruct,
                    group_ptr,
                    data_ptr,
                );
            }

            for (func_index, function) in block_ctx.layout.functions.iter().enumerate() {
                let layout_index = block_ctx.layout.function_index(func_index);
                let data_ptr = block_ctx.get_data_entry(layout_index);
                functions::build_lifecycle_call(
                    module,
                    &mut block_ctx.ctx.b,
                    function,
                    functions::FunctionLifecycleFunc::Destruct,
                    data_ptr,
                );
            }
        },
    )
}

pub fn build_lifecycle_call(
    module: &Module,
    builder: &mut Builder,
    block: &Block,
    target: &TargetProperties,
    lifecycle: LifecycleFunc,
    scratch_ptr: PointerValue,
    groups_ptr: PointerValue,
    ui_ptr: Option<PointerValue>,
) {
    let func = get_lifecycle_func(module, block, target, lifecycle);
    if let Some(ui_ptr) = ui_ptr {
        builder.build_call(&func, &[&scratch_ptr, &groups_ptr, &ui_ptr], "", false);
    } else {
        builder.build_call(&func, &[&scratch_ptr, &groups_ptr], "", false);
    }
}
