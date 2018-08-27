use super::ControlContext;
use super::{default_copy_getter, default_copy_setter, Control, ControlFieldGenerator};
use ast::{ControlField, ControlType, GraphField};
use inkwell::context::Context;
use inkwell::types::StructType;
use inkwell::values::PointerValue;

pub struct GraphControl;
impl Control for GraphControl {
    fn control_type() -> ControlType {
        ControlType::Graph
    }

    fn data_type(context: &Context) -> StructType {
        context.struct_type(
            &[
                &context.i8_type(),                 // curve count
                &context.f32_type().array_type(17), // start values
                &context.f32_type().array_type(16), // end positions
                &context.f32_type().array_type(16), // tension
                &context.i8_type().array_type(17),  // states
            ],
            false,
        )
    }

    fn gen_construct(control: &mut ControlContext) {
        let count_ptr = unsafe { control.ctx.b.build_struct_gep(&control.data_ptr, 0, "") };
        let start_vals_array_ptr =
            unsafe { control.ctx.b.build_struct_gep(&control.data_ptr, 1, "") };
        let end_positions_array_ptr =
            unsafe { control.ctx.b.build_struct_gep(&control.data_ptr, 2, "") };
        let tension_array_ptr = unsafe { control.ctx.b.build_struct_gep(&control.data_ptr, 3, "") };
        let state_array_ptr = unsafe { control.ctx.b.build_struct_gep(&control.data_ptr, 4, "") };

        control.ctx.b.build_store(
            &count_ptr,
            &control.ctx.context.i8_type().const_int(2, false),
        );

        control.ctx.b.build_store(
            &unsafe { control.ctx.b.build_struct_gep(&start_vals_array_ptr, 0, "") },
            &control.ctx.context.f32_type().const_float(0.),
        );
        control.ctx.b.build_store(
            &unsafe {
                control
                    .ctx
                    .b
                    .build_struct_gep(&end_positions_array_ptr, 0, "")
            },
            &control.ctx.context.f32_type().const_float(1.),
        );
        control.ctx.b.build_store(
            &unsafe { control.ctx.b.build_struct_gep(&tension_array_ptr, 0, "") },
            &control.ctx.context.f32_type().const_float(-0.5),
        );
        control.ctx.b.build_store(
            &unsafe { control.ctx.b.build_struct_gep(&state_array_ptr, 0, "") },
            &control.ctx.context.i8_type().const_int(0, true),
        );

        control.ctx.b.build_store(
            &unsafe { control.ctx.b.build_struct_gep(&start_vals_array_ptr, 1, "") },
            &control.ctx.context.f32_type().const_float(1.),
        );
        control.ctx.b.build_store(
            &unsafe {
                control
                    .ctx
                    .b
                    .build_struct_gep(&end_positions_array_ptr, 1, "")
            },
            &control.ctx.context.f32_type().const_float(1.5),
        );
        control.ctx.b.build_store(
            &unsafe { control.ctx.b.build_struct_gep(&tension_array_ptr, 1, "") },
            &control.ctx.context.f32_type().const_float(0.75),
        );
        control.ctx.b.build_store(
            &unsafe { control.ctx.b.build_struct_gep(&state_array_ptr, 1, "") },
            &control.ctx.context.i8_type().const_int(1, true),
        );

        control.ctx.b.build_store(
            &unsafe { control.ctx.b.build_struct_gep(&start_vals_array_ptr, 2, "") },
            &control.ctx.context.f32_type().const_float(0.7),
        );
    }

    fn gen_fields(generator: &ControlFieldGenerator) {
        generator.generate(
            ControlField::Graph(GraphField::Value),
            &default_copy_getter,
            &default_copy_setter,
        );
        generator.generate(
            ControlField::Graph(GraphField::Speed),
            &speed_field_getter,
            &speed_field_setter,
        );
    }
}

fn speed_field_getter(_control: &mut ControlContext, _out_val: PointerValue) {
    // todo
}

fn speed_field_setter(_control: &mut ControlContext, _in_val: PointerValue) {
    // todo
}
