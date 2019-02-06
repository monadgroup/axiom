use crate::codegen::controls::ControlPointers;
use crate::codegen::data_analyzer::BlockLayout;
use crate::codegen::BuilderContext;
use inkwell::values::PointerValue;

pub struct BlockContext<'a> {
    pub ctx: BuilderContext<'a>,
    pub layout: &'a BlockLayout,
    statement_ptrs: Vec<PointerValue>,
    pointers_ptr: PointerValue,
}

impl<'a> BlockContext<'a> {
    pub fn new(
        ctx: BuilderContext<'a>,
        layout: &'a BlockLayout,
        pointers_ptr: PointerValue,
    ) -> Self {
        BlockContext {
            ctx,
            layout,
            statement_ptrs: Vec::new(),
            pointers_ptr,
        }
    }

    pub fn push_statement(&mut self, ptr: PointerValue) {
        self.statement_ptrs.push(ptr)
    }

    pub fn get_statement(&self, index: usize) -> PointerValue {
        self.statement_ptrs[index]
    }

    pub fn get_control_ptrs(&self, index: usize) -> ControlPointers {
        let layout_index = self.layout.control_index(index);
        let base_ptr = unsafe {
            self.ctx
                .b
                .build_struct_gep(&self.pointers_ptr, layout_index as u32, "ctx.control")
        };
        ControlPointers {
            group: self
                .ctx
                .b
                .build_load(
                    &unsafe {
                        self.ctx
                            .b
                            .build_struct_gep(&base_ptr, 0, "ctx.control.value.ptr")
                    },
                    "ctx.control.value",
                )
                .into_pointer_value(),
            data: self
                .ctx
                .b
                .build_load(
                    &unsafe {
                        self.ctx
                            .b
                            .build_struct_gep(&base_ptr, 1, "ctx.control.data.ptr")
                    },
                    "ctx.control.data",
                )
                .into_pointer_value(),
            shared: self
                .ctx
                .b
                .build_load(
                    &unsafe {
                        self.ctx
                            .b
                            .build_struct_gep(&base_ptr, 2, "ctx.control.shared.ptr")
                    },
                    "ctx.control.shared",
                )
                .into_pointer_value(),
        }
    }

    pub fn get_ui_ptr(&self, index: usize) -> PointerValue {
        let layout_index = self.layout.control_index(index);
        let base_ptr = unsafe {
            self.ctx
                .b
                .build_struct_gep(&self.pointers_ptr, layout_index as u32, "ctx.control")
        };

        self.ctx
            .b
            .build_load(
                &unsafe {
                    self.ctx
                        .b
                        .build_struct_gep(&base_ptr, 3, "ctx.control.ui.ptr")
                },
                "ctx.control.ui",
            )
            .into_pointer_value()
    }

    pub fn get_function_ptr(&self, layout_index: usize) -> PointerValue {
        self.ctx
            .b
            .build_load(
                &unsafe {
                    self.ctx.b.build_struct_gep(
                        &self.pointers_ptr,
                        layout_index as u32,
                        "ctx.function.ptr",
                    )
                },
                "ctx.function",
            )
            .into_pointer_value()
    }
}
