use ast::{Form, UnaryOperation, PostfixOperation, OperatorType, ControlField};
use mir::block::Function;

#[derive(Debug, Clone)]
pub enum Statement {
    NumConstant { left: f32, right: f32, form: Form },
    NumConvert { target_form: Form, input: usize },
    NumCast { target_form: Form, input: usize },
    NumUnaryOp { op: UnaryOperation, input: usize },
    NumPostfixOp { op: PostfixOperation, input: usize },
    NumMathOp { op: OperatorType, left: usize, right: usize },

    CallFunc { function: Function, args: Vec<usize> },
    StoreControl { control: usize, field: ControlField, value: usize },
    LoadControl { control: usize, field: ControlField }
}
