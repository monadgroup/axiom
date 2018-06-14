use ast::{ControlField, Form, OperatorType, PostfixOperation, SourceRange, UnaryOperation};

#[derive(Debug)]
pub struct KnownExpression<T> {
    pub pos: SourceRange,
    pub data: T,
}

pub type SubExpression = Box<Expression>;

#[derive(Debug)]
pub struct AssignExpression {
    pub left: KnownExpression<LValueExpression>,
    pub right: SubExpression,
    pub operator: OperatorType,
}

#[derive(Debug)]
pub struct CallExpression {
    pub name: String,
    pub arguments: Vec<Expression>,
}

#[derive(Debug)]
pub struct CastExpression {
    pub target: Form,
    pub expr: SubExpression,
    pub is_convert: bool,
}

#[derive(Debug)]
pub struct ControlExpression {
    pub name: String,
    pub field: ControlField,
}

#[derive(Debug)]
pub struct LValueExpression {
    pub assignments: Vec<AssignableExpression>,
}

#[derive(Debug)]
pub struct MathExpression {
    pub left: SubExpression,
    pub right: SubExpression,
    pub operator: OperatorType,
}

#[derive(Debug)]
pub struct NoteExpression {
    pub note: i32,
}

#[derive(Debug)]
pub struct NumberExpression {
    pub value: f32,
    pub form: Form,
}

#[derive(Debug)]
pub struct PostfixExpression {
    pub left: KnownExpression<LValueExpression>,
    pub operation: PostfixOperation,
}

#[derive(Debug)]
pub struct TupleExpression {
    pub expressions: Vec<Expression>,
}

#[derive(Debug)]
pub struct UnaryExpression {
    pub operation: UnaryOperation,
    pub expr: SubExpression,
}

#[derive(Debug)]
pub struct VariableExpression {
    pub name: String,
}

#[derive(Debug)]
pub enum ExpressionData {
    Assign(AssignExpression),
    Call(CallExpression),
    Cast(CastExpression),
    Control(ControlExpression),
    LValue(LValueExpression),
    Math(MathExpression),
    Note(NoteExpression),
    Number(NumberExpression),
    Postfix(PostfixExpression),
    Tuple(TupleExpression),
    Unary(UnaryExpression),
    Variable(VariableExpression),
}

#[derive(Debug)]
pub enum AssignableData {
    Control(ControlExpression),
    Variable(VariableExpression),
}

#[derive(Debug)]
pub struct Expression {
    pub pos: SourceRange,
    pub data: ExpressionData,
}

#[derive(Debug)]
pub struct AssignableExpression {
    pub pos: SourceRange,
    pub data: AssignableData,
}

impl Expression {
    pub fn new(pos: SourceRange, data: ExpressionData) -> Expression {
        Expression { pos, data }
    }

    pub fn new_assign(
        pos: SourceRange,
        left: KnownExpression<LValueExpression>,
        right: SubExpression,
        operator: OperatorType,
    ) -> Expression {
        Expression::new(
            pos,
            ExpressionData::Assign(AssignExpression {
                left,
                right,
                operator,
            }),
        )
    }

    pub fn new_call(pos: SourceRange, name: String, arguments: Vec<Expression>) -> Expression {
        Expression::new(
            pos,
            ExpressionData::Call(CallExpression { name, arguments }),
        )
    }

    pub fn new_cast(
        pos: SourceRange,
        target: Form,
        expr: SubExpression,
        is_convert: bool,
    ) -> Expression {
        Expression::new(
            pos,
            ExpressionData::Cast(CastExpression {
                target,
                expr,
                is_convert,
            }),
        )
    }

    pub fn new_control(
        pos: SourceRange,
        name: String,
        field: ControlField,
    ) -> Expression {
        Expression::new(
            pos,
            ExpressionData::Control(ControlExpression {
                name,
                field,
            }),
        )
    }

    pub fn new_lvalue(pos: SourceRange, assignments: Vec<AssignableExpression>) -> Expression {
        Expression::new(
            pos,
            ExpressionData::LValue(LValueExpression { assignments }),
        )
    }

    pub fn new_math(
        pos: SourceRange,
        left: SubExpression,
        right: SubExpression,
        operator: OperatorType,
    ) -> Expression {
        Expression::new(
            pos,
            ExpressionData::Math(MathExpression {
                left,
                right,
                operator,
            }),
        )
    }

    pub fn new_note(pos: SourceRange, note: i32) -> Expression {
        Expression::new(pos, ExpressionData::Note(NoteExpression { note }))
    }

    pub fn new_number(pos: SourceRange, value: f32, form: Form) -> Expression {
        Expression::new(
            pos,
            ExpressionData::Number(NumberExpression { value, form }),
        )
    }

    pub fn new_postfix(
        pos: SourceRange,
        left: KnownExpression<LValueExpression>,
        operation: PostfixOperation,
    ) -> Expression {
        Expression::new(
            pos,
            ExpressionData::Postfix(PostfixExpression { left, operation }),
        )
    }

    pub fn new_tuple(pos: SourceRange, expressions: Vec<Expression>) -> Expression {
        Expression::new(pos, ExpressionData::Tuple(TupleExpression { expressions }))
    }

    pub fn new_unary(
        pos: SourceRange,
        operation: UnaryOperation,
        expr: SubExpression,
    ) -> Expression {
        Expression::new(
            pos,
            ExpressionData::Unary(UnaryExpression { operation, expr }),
        )
    }

    pub fn new_variable(pos: SourceRange, name: String) -> Expression {
        Expression::new(pos, ExpressionData::Variable(VariableExpression { name }))
    }
}

impl<T> KnownExpression<T> {
    pub fn new(pos: SourceRange, data: T) -> KnownExpression<T> {
        KnownExpression { pos, data }
    }
}

impl AssignableExpression {
    pub fn new(pos: SourceRange, data: AssignableData) -> AssignableExpression {
        AssignableExpression { pos, data }
    }

    pub fn new_variable(pos: SourceRange, expr: VariableExpression) -> AssignableExpression {
        AssignableExpression::new(pos, AssignableData::Variable(expr))
    }

    pub fn new_control(pos: SourceRange, expr: ControlExpression) -> AssignableExpression {
        AssignableExpression::new(pos, AssignableData::Control(expr))
    }

    pub fn from(expr: Expression) -> Option<AssignableExpression> {
        let data = match expr.data {
            ExpressionData::Control(control) => AssignableData::Control(control),
            ExpressionData::Variable(var) => AssignableData::Variable(var),
            _ => return None,
        };
        Some(AssignableExpression::new(expr.pos, data))
    }
}
