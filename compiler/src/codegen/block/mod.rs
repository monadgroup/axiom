mod block_context;
mod gen_call_func;
mod gen_combine;
mod gen_constant;
mod gen_extract;
mod gen_global;
mod gen_load_control;
mod gen_math_op;
mod gen_num_cast;
mod gen_num_convert;
mod gen_store_control;
mod gen_unary_op;

use self::block_context::BlockContext;
use crate::codegen::{
    build_context_function, controls, functions, util, BuilderContext, LifecycleFunc, ObjectCache,
};
use crate::mir::block::Statement;
use crate::mir::{Block, BlockRef};
use inkwell::builder::Builder;
use inkwell::module::{Linkage, Module};
use inkwell::values::{FunctionValue, PointerValue};
use inkwell::AddressSpace;

use self::gen_call_func::gen_call_func_statement;
use self::gen_combine::gen_combine_statement;
use self::gen_constant::gen_constant_statement;
use self::gen_extract::gen_extract_statement;
use self::gen_global::gen_global_statement;
use self::gen_load_control::gen_load_control_statement;
use self::gen_math_op::gen_math_op_statement;
use self::gen_num_cast::gen_num_cast_statement;
use self::gen_num_convert::gen_num_convert_statement;
use self::gen_store_control::gen_store_control_statement;
use self::gen_unary_op::gen_unary_op_statement;

fn gen_statement(index: usize, statement: &Statement, node: &mut BlockContext) -> PointerValue {
    match statement {
        Statement::Constant(constant) => gen_constant_statement(constant, node),
        Statement::Global(global) => gen_global_statement(global, node),
        Statement::NumConvert { target_form, input } => {
            gen_num_convert_statement(target_form, *input, node)
        }
        Statement::NumCast { target_form, input } => {
            gen_num_cast_statement(target_form, *input, node)
        }
        Statement::NumUnaryOp { op, input } => gen_unary_op_statement(op, *input, node),
        Statement::NumMathOp { op, lhs, rhs } => gen_math_op_statement(op, *lhs, *rhs, node),
        Statement::Extract { tuple, index } => gen_extract_statement(*tuple, *index, node),
        Statement::Combine { indexes } => gen_combine_statement(indexes, node),
        Statement::CallFunc {
            function,
            args,
            varargs,
        } => gen_call_func_statement(index, function, args, varargs, node),
        Statement::StoreControl {
            control,
            field,
            value,
        } => gen_store_control_statement(*control, field, *value, node),
        Statement::LoadControl { control, field } => {
            gen_load_control_statement(*control, field, node)
        }
    }
}

fn get_lifecycle_func(
    module: &Module,
    cache: &ObjectCache,
    block: BlockRef,
    lifecycle: LifecycleFunc,
) -> FunctionValue {
    let func_name = format!("maxim.block.{}.{}", block, lifecycle);
    util::get_or_create_func(module, &func_name, true, &|| {
        let context = module.get_context();
        let layout = cache.block_layout(block).unwrap();

        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(
                &[&layout.pointer_struct.ptr_type(AddressSpace::Generic)],
                false,
            ),
        )
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
    cache: &ObjectCache,
    block: BlockRef,
    lifecycle: LifecycleFunc,
    cb: &Fn(&mut BlockContext),
) {
    let func = get_lifecycle_func(module, cache, block, lifecycle);
    build_context_function(module, func, cache.target(), &|ctx: BuilderContext| {
        let layout = cache.block_layout(block).unwrap();
        let pointers_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();
        let mut ctx = BlockContext::new(ctx, layout, pointers_ptr);
        cb(&mut ctx);
        ctx.ctx.b.build_return(None);
    });
}

pub fn build_construct_func(module: &Module, cache: &ObjectCache, block: &Block) {
    build_lifecycle_func(
        module,
        cache,
        block.id.id,
        LifecycleFunc::Construct,
        &|block_ctx: &mut BlockContext| {
            for (control_index, control) in block.controls.iter().enumerate() {
                let ptrs = block_ctx.get_control_ptrs(control_index, cache.target().include_ui);
                controls::build_lifecycle_call(
                    module,
                    &mut block_ctx.ctx.b,
                    control.control_type,
                    LifecycleFunc::Construct,
                    ptrs.value,
                    ptrs.data,
                    ptrs.shared,
                );

                if let Some(ui_ptr) = ptrs.ui {
                    controls::build_ui_lifecycle_call(
                        module,
                        &mut block_ctx.ctx.b,
                        control.control_type,
                        LifecycleFunc::Construct,
                        ptrs.value,
                        ptrs.data,
                        ptrs.shared,
                        ui_ptr,
                    );
                }
            }

            for (func_index, function) in block_ctx.layout.functions.iter().enumerate() {
                let layout_index = block_ctx.layout.function_index(func_index);
                let data_ptr = block_ctx.get_function_ptr(layout_index);
                functions::build_lifecycle_call(
                    module,
                    &mut block_ctx.ctx.b,
                    *function,
                    functions::FunctionLifecycleFunc::Construct,
                    data_ptr,
                );
            }
        },
    )
}

pub fn build_update_func(module: &Module, cache: &ObjectCache, block: &Block) {
    build_lifecycle_func(
        module,
        cache,
        block.id.id,
        LifecycleFunc::Update,
        &|block_ctx: &mut BlockContext| {
            for (control_index, control) in block.controls.iter().enumerate() {
                let ptrs = block_ctx.get_control_ptrs(control_index, cache.target().include_ui);
                controls::build_lifecycle_call(
                    module,
                    &mut block_ctx.ctx.b,
                    control.control_type,
                    LifecycleFunc::Update,
                    ptrs.value,
                    ptrs.data,
                    ptrs.shared,
                );

                if let Some(ui_ptr) = ptrs.ui {
                    controls::build_ui_lifecycle_call(
                        module,
                        &mut block_ctx.ctx.b,
                        control.control_type,
                        LifecycleFunc::Update,
                        ptrs.value,
                        ptrs.data,
                        ptrs.shared,
                        ui_ptr,
                    );
                }
            }

            for (statement_index, statement) in block.statements.iter().enumerate() {
                let statement_result = gen_statement(statement_index, statement, block_ctx);
                block_ctx.push_statement(statement_result);
            }
        },
    )
}

pub fn build_destruct_func(module: &Module, cache: &ObjectCache, block: &Block) {
    build_lifecycle_func(
        module,
        cache,
        block.id.id,
        LifecycleFunc::Destruct,
        &|block_ctx: &mut BlockContext| {
            for (control_index, control) in block.controls.iter().enumerate() {
                let ptrs = block_ctx.get_control_ptrs(control_index, cache.target().include_ui);

                if let Some(ui_ptr) = ptrs.ui {
                    controls::build_ui_lifecycle_call(
                        module,
                        &mut block_ctx.ctx.b,
                        control.control_type,
                        LifecycleFunc::Destruct,
                        ptrs.value,
                        ptrs.data,
                        ptrs.shared,
                        ui_ptr,
                    );
                }

                controls::build_lifecycle_call(
                    module,
                    &mut block_ctx.ctx.b,
                    control.control_type,
                    LifecycleFunc::Destruct,
                    ptrs.value,
                    ptrs.data,
                    ptrs.shared,
                );
            }

            for (func_index, function) in block_ctx.layout.functions.iter().enumerate() {
                let layout_index = block_ctx.layout.function_index(func_index);
                let data_ptr = block_ctx.get_function_ptr(layout_index);
                functions::build_lifecycle_call(
                    module,
                    &mut block_ctx.ctx.b,
                    *function,
                    functions::FunctionLifecycleFunc::Destruct,
                    data_ptr,
                );
            }
        },
    )
}

pub fn build_funcs(module: &Module, cache: &ObjectCache, block: &Block) {
    build_construct_func(module, cache, block);
    build_update_func(module, cache, block);
    build_destruct_func(module, cache, block);
}

pub fn build_lifecycle_call(
    module: &Module,
    cache: &ObjectCache,
    builder: &mut Builder,
    block: BlockRef,
    lifecycle: LifecycleFunc,
    pointers_ptr: PointerValue,
) {
    let func = get_lifecycle_func(module, cache, block, lifecycle);
    builder.build_call(&func, &[&pointers_ptr], "", true);
}
