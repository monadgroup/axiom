#[derive(Debug, Clone, Copy)]
pub enum Function {

}

pub struct FunctionData {
    pub has_side_effects: bool
}

impl Function {
    pub fn data(&self) -> &FunctionData {
        unimplemented!();
    }

    pub fn has_side_effects(&self) -> bool {
        self.data().has_side_effects
    }
}
