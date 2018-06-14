use ast;
use mir;
use std::collections::HashMap;
use {CompileError, CompileResult};

// lowers an AST into a Block MIR object
pub fn lower_ast(id: mir::BlockId, block: &ast::Block) -> mir::Block {
    let mut lower = AstLower::new(id);
    for expr in &block.expressions {
        lower.lower_expression(expr);
    }
    lower.block
}

struct AstLower<'a> {
    pub block: mir::Block,
    var_indexes: HashMap<&'a str, usize>
}

type LowerResult = CompileResult<usize>;

impl<'a> AstLower<'a> {
    pub fn new(id: mir::BlockId) -> AstLower<'a> {
        AstLower {
            block: mir::Block::new(id, Vec::new(), Vec::new()),
            var_indexes: HashMap::new()
        }
    }

    pub fn lower_expression(&mut self, expr: &'a ast::Expression) -> LowerResult {
        match &expr.data {
            ast::ExpressionData::Assign(ref assign) => self.lower_assign_expr(&expr.pos, assign),
            _ => unimplemented!()
        }
    }

    fn lower_assign_expr(&mut self, pos: &ast::SourceRange, expr: &'a ast::AssignExpression) -> LowerResult {
        let rhs = match self.lower_expression(&expr.right) {
            Ok(v) => v,
            Err(err) => return Err(err)
        };

        // assignments work in the following way:
        //  - if both sides are tuples, assign each item in the left to one on the right (they must be the same size!)
        //  - if only the right is a tuple, assign the tuple to the item on the left
        //  - if only the left is a tuple, assign the item on the right to all items on the left
        // the return value is _always_ what was on the RHS!

        let left_amt = expr.left.data.assignments.len();
        if left_amt > 1 {
            match mir::VarType::of_statement(&self.block, rhs) {
                mir::VarType::Tuple(types) => {
                    if left_amt != types.len() {
                        return Err(CompileError::unmatched_tuples(left_amt, types.len(), expr.left.pos))
                    }

                    for i in 0..types.len() {
                        // extract the item out of the tuple and set
                        let extracted_index = self.block.statements.len();
                        self.block.statements.push(mir::block::Statement::ExtractOp {
                            tuple: rhs,
                            index: i
                        });
                        if let Some(err) = self.set_assignable(&expr.left.data.assignments[i], extracted_index) {
                            return Err(err)
                        }
                    }
                },
                _ => {
                    for assignment in &expr.left.data.assignments {
                        if let Some(err) = self.set_assignable(assignment, rhs) {
                            return Err(err)
                        }
                    }
                }
            }
        } else {
            if let Some(err) = self.set_assignable(&expr.left.data.assignments[0], rhs) {
                return Err(err)
            }
        }

        Ok(rhs)
    }

    fn set_assignable(&mut self, expr: &'a ast::AssignableExpression, value: usize) -> Option<CompileError> {
        match expr.data {
            ast::AssignableData::Control(ref data) => {
                let expected_type = mir::VarType::of_control_field(data.field);
                let actual_type = mir::VarType::of_statement(&self.block, value);
                if expected_type != actual_type {
                    Some(CompileError::mismatched_type(expected_type, actual_type, expr.pos))
                } else {
                    let control = self.get_control_index(data);
                    self.block.statements.push(mir::block::Statement::StoreControl {
                        control,
                        field: data.field,
                        value
                    });
                    None
                }
            },
            ast::AssignableData::Variable(ref expr) => {
                // variables are SSA, no typechecking needed
                self.var_indexes.insert(&expr.name, value);
                None
            }
        }
    }

    fn get_control_index(&mut self, expr: &ast::ControlExpression) -> usize {
        unimplemented!();
    }
}
