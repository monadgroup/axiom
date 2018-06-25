use super::data_analyzer::BlockLayout;
use super::BuilderContext;
use inkwell::values::PointerValue;

pub struct BlockContext<'a> {
    pub ctx: BuilderContext<'a>,
    pub layout: BlockLayout,
    statement_ptrs: Vec<PointerValue>,
    data_ptr: PointerValue,
    group_ptr: PointerValue,
    ui_ptr: Option<PointerValue>,
}

impl<'a> BlockContext<'a> {
    pub fn get_statement(&self, index: usize) -> PointerValue {
        self.statement_ptrs[index]
    }

    pub fn get_data_entry(&self, index: usize) -> PointerValue {
        unsafe {
            self.ctx
                .b
                .build_struct_gep(&self.data_ptr, index as u32, "ctx.dataentry")
        }
    }

    pub fn get_group_entry(&self, index: usize) -> PointerValue {
        unsafe {
            self.ctx
                .b
                .build_struct_gep(&self.group_ptr, index as u32, "ctx.groupentry")
        }
    }

    pub fn get_ui_entry(&self, index: usize) -> Option<PointerValue> {
        match self.ui_ptr {
            Some(val) => unsafe {
                Some(
                    self.ctx
                        .b
                        .build_struct_gep(&val, index as u32, "ctx.uientry"),
                )
            },
            None => None,
        }
    }
}
