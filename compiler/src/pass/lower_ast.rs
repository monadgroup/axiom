use ast;
use mir;
use std::collections::HashMap;
use util::constant_propagate;
use {CompileError, CompileResult};

// lowers an AST into a Block MIR object
pub fn lower_ast(id: mir::BlockId, block: &ast::Block) -> CompileResult<mir::Block> {
    let mut lower = AstLower::new(id);
    for expr in &block.expressions {
        if let Err(err) = lower.lower_expression(expr) {
            return Err(err);
        }
    }
    Ok(lower.block)
}

struct AstLower<'a> {
    pub block: mir::Block,
    var_indexes: HashMap<&'a str, usize>,
    control_indexes: HashMap<(&'a str, ast::ControlType), usize>,
}

type LowerResult = CompileResult<usize>;

impl<'a> AstLower<'a> {
    pub fn new(id: mir::BlockId) -> AstLower<'a> {
        AstLower {
            block: mir::Block::new(id, Vec::new(), Vec::new()),
            var_indexes: HashMap::new(),
            control_indexes: HashMap::new(),
        }
    }

    pub fn lower_expression(&mut self, expr: &'a ast::Expression) -> LowerResult {
        match &expr.data {
            ast::ExpressionData::Assign(ref assign) => self.lower_assign_expr(&expr.pos, assign),
            ast::ExpressionData::Call(ref call) => self.lower_call_expr(&expr.pos, call),
            ast::ExpressionData::Cast(ref cast) => self.lower_cast_expr(&expr.pos, cast),
            ast::ExpressionData::Control(ref control) => self.lower_control_expr(control),
            ast::ExpressionData::Math(ref math) => self.lower_math_expr(&expr.pos, math),
            ast::ExpressionData::Note(ref note) => self.lower_note_expr(note),
            ast::ExpressionData::Number(ref num) => self.lower_number_expr(num),
            ast::ExpressionData::Postfix(ref postfix) => {
                self.lower_postfix_expr(&expr.pos, postfix)
            }
            ast::ExpressionData::Tuple(ref tuple) => self.lower_tuple_expr(tuple),
            ast::ExpressionData::Unary(ref unary) => self.lower_unary_expr(&expr.pos, unary),
            ast::ExpressionData::Variable(ref variable) => {
                self.lower_variable_expr(&expr.pos, variable)
            }
        }
    }

    fn lower_assignable(&mut self, expr: &'a ast::AssignableExpression) -> LowerResult {
        match &expr.data {
            ast::AssignableData::Control(ref control) => self.lower_control_expr(control),
            ast::AssignableData::Variable(ref variable) => {
                self.lower_variable_expr(&expr.pos, variable)
            }
        }
    }

    fn lower_assign_expr(
        &mut self,
        pos: &ast::SourceRange,
        expr: &'a ast::AssignExpression,
    ) -> LowerResult {
        let rhs = match self.lower_expression(&expr.right) {
            Ok(v) => v,
            Err(err) => return Err(err),
        };
        let right_values = match self.get_expr_values(&expr.right.pos, rhs) {
            Ok(v) => v,
            Err(err) => return Err(err),
        };

        match self.lower_assign(&expr.left, right_values) {
            Some(err) => Err(err),
            None => Ok(rhs),
        }
    }

    fn lower_assign(
        &mut self,
        lvalue: &'a ast::KnownExpression<ast::LValueExpression>,
        right_vals: Vec<usize>,
    ) -> Option<CompileError> {
        // assignments work in the following way:
        //  - if both sides are tuples, assign each item in the left to one on the right (they must be the same size!)
        //  - if only the right is a tuple, assign the tuple to the item on the left
        //  - if only the left is a tuple, assign the item on the right to all items on the left
        // the return value is _always_ what was on the RHS!

        if lvalue.data.assignments.len() == 1 {
            let tuple = self.squash_values(right_vals);
            if let Some(err) = self.set_assignable(&lvalue.data.assignments[0], tuple) {
                return Some(err);
            }
        } else if right_vals.len() == 1 {
            for assignment in &lvalue.data.assignments {
                if let Some(err) = self.set_assignable(assignment, right_vals[0]) {
                    return Some(err);
                }
            }
        } else {
            if lvalue.data.assignments.len() != right_vals.len() {
                return Some(CompileError::unmatched_tuples(
                    lvalue.data.assignments.len(),
                    right_vals.len(),
                    lvalue.pos.clone(),
                ));
            }

            for (i, rhs) in right_vals.iter().enumerate() {
                if let Some(err) = self.set_assignable(&lvalue.data.assignments[i], *rhs) {
                    return Some(err);
                }
            }
        }

        None
    }

    fn set_assignable(
        &mut self,
        expr: &'a ast::AssignableExpression,
        value: usize,
    ) -> Option<CompileError> {
        match expr.data {
            ast::AssignableData::Control(ref data) => {
                let control = self.get_control_index(data);
                match self.add_store_control(&expr.pos, control, data.field, value) {
                    Ok(_) => None,
                    Err(err) => Some(err),
                }
            }
            ast::AssignableData::Variable(ref expr) => {
                self.var_indexes.insert(&expr.name, value);
                None
            }
        }
    }

    fn lower_call_expr(
        &mut self,
        pos: &ast::SourceRange,
        expr: &'a ast::CallExpression,
    ) -> LowerResult {
        // todo
        // function overloads are handled here, we also constant-propagate if all arguments are
        // constants and the function supports it.

        unimplemented!();
    }

    fn lower_cast_expr(
        &mut self,
        pos: &ast::SourceRange,
        expr: &'a ast::CastExpression,
    ) -> LowerResult {
        let rhs = match self.lower_expression(expr.expr.as_ref()) {
            Ok(v) => v,
            Err(err) => return Err(err),
        };

        // if the RHS is a tuple, iterate over each item, cast it individually, then combine all items back again
        if let mir::VarType::Tuple(subtypes) = mir::VarType::of_statement(&self.block, rhs) {
            let converted_indices: CompileResult<Vec<_>> = (0..subtypes.len())
                .map(|index| {
                    let extract_index = self.add_extract_op(pos, rhs, index);
                    extract_index.and_then(|index| {
                        self.lower_cast(pos, index, expr.target.form_type, expr.is_convert)
                    })
                })
                .collect();

            match converted_indices {
                Ok(indices) => Ok(self.add_combine_op(indices)),
                Err(err) => Err(err),
            }
        } else {
            self.lower_cast(pos, rhs, expr.target.form_type, expr.is_convert)
        }
    }

    fn lower_cast(
        &mut self,
        pos: &ast::SourceRange,
        rhs: usize,
        target_form: ast::FormType,
        is_convert: bool,
    ) -> LowerResult {
        if is_convert {
            self.add_num_convert(pos, target_form, rhs)
        } else {
            self.add_num_cast(pos, target_form, rhs)
        }
    }

    fn lower_control_expr(&mut self, expr: &'a ast::ControlExpression) -> LowerResult {
        let index = self.get_control_index(expr);
        self.block.controls[index].value_read = true;
        Ok(self.add_load_control(index, expr.field))
    }

    fn lower_math_expr(
        &mut self,
        pos: &ast::SourceRange,
        expr: &'a ast::MathExpression,
    ) -> LowerResult {
        // if both sides are a tuple, they must be the same size, we add piece-wise and combine the result
        // if only one side is a tuple, we "spread" that tuple out

        let left_values = match self.lower_expression(expr.left.as_ref())
            .and_then(|value| self.get_expr_values(&expr.left.pos, value))
        {
            Ok(v) => v,
            Err(err) => return Err(err),
        };
        let right_values = match self.lower_expression(expr.right.as_ref())
            .and_then(|value| self.get_expr_values(&expr.right.pos, value))
        {
            Ok(v) => v,
            Err(err) => return Err(err),
        };

        match self.lower_math_op(pos, &left_values, &right_values, expr.operator) {
            Ok(vals) => Ok(self.squash_values(vals)),
            Err(err) => Err(err),
        }
    }

    fn get_expr_values(
        &mut self,
        pos: &ast::SourceRange,
        value: usize,
    ) -> CompileResult<Vec<usize>> {
        match mir::VarType::of_statement(&self.block, value) {
            mir::VarType::Tuple(items) => (0..items.len())
                .map(|index| self.add_extract_op(pos, value, index))
                .collect(),
            _ => Ok(vec![value]),
        }
    }

    fn squash_values(&mut self, values: Vec<usize>) -> usize {
        if values.len() == 1 {
            values[0]
        } else {
            self.add_combine_op(values)
        }
    }

    fn lower_math_op(
        &mut self,
        pos: &ast::SourceRange,
        left_items: &Vec<usize>,
        right_items: &Vec<usize>,
        op: ast::OperatorType,
    ) -> CompileResult<Vec<usize>> {
        if left_items.len() != 1 && right_items.len() != 1 {
            // both sides are tuples, they should be the same size
            if left_items.len() != right_items.len() {
                return Err(CompileError::unmatched_tuples(
                    left_items.len(),
                    right_items.len(),
                    pos.clone(),
                ));
            }

            (0..left_items.len())
                .map(|index| self.add_num_math_op(pos, op, left_items[index], right_items[index]))
                .collect()
        } else {
            // only one side is a tuple
            let iter_count = left_items.len().max(right_items.len());
            (0..iter_count)
                .map(|index| {
                    let left_index = if index >= left_items.len() { 0 } else { index };
                    let right_index = if index >= right_items.len() { 0 } else { index };
                    self.add_num_math_op(pos, op, left_items[left_index], right_items[right_index])
                })
                .collect()
        }
    }

    fn lower_note_expr(&mut self, expr: &'a ast::NoteExpression) -> LowerResult {
        Ok(self.add_statement(mir::block::Statement::new_const_num(
            mir::block::ConstantNum::new(expr.note as f32, expr.note as f32, ast::FormType::Note),
        )))
    }

    fn lower_number_expr(&mut self, expr: &'a ast::NumberExpression) -> LowerResult {
        Ok(self.add_statement(mir::block::Statement::new_const_num(
            mir::block::ConstantNum::new(expr.value, expr.value, expr.form.form_type),
        )))
    }

    fn lower_postfix_expr(
        &mut self,
        pos: &ast::SourceRange,
        expr: &'a ast::PostfixExpression,
    ) -> LowerResult {
        let maybe_vals: CompileResult<Vec<_>> = expr.left
            .data
            .assignments
            .iter()
            .map(|assignment| self.lower_assignable(assignment))
            .collect();

        let vals = match maybe_vals {
            Ok(vals) => vals,
            Err(err) => return Err(err),
        };
        let one_constant = self.add_statement(mir::block::Statement::new_const_num(
            mir::block::ConstantNum::new(1., 1., ast::FormType::None),
        ));
        let op_type = match expr.operation {
            ast::PostfixOperation::Increment => ast::OperatorType::Add,
            ast::PostfixOperation::Decrement => ast::OperatorType::Subtract,
        };
        let op_results = match self.lower_math_op(pos, &vals, &vec![one_constant], op_type) {
            Ok(results) => results,
            Err(err) => return Err(err),
        };
        if let Some(err) = self.lower_assign(&expr.left, op_results) {
            Err(err)
        } else {
            Ok(self.squash_values(vals))
        }
    }

    fn lower_unary_expr(
        &mut self,
        pos: &ast::SourceRange,
        expr: &'a ast::UnaryExpression,
    ) -> LowerResult {
        let value = match self.lower_expression(expr.expr.as_ref()) {
            Ok(v) => v,
            Err(err) => return Err(err),
        };

        let results = self.get_expr_values(pos, value).and_then(|values| {
            values
                .iter()
                .map(|value| self.add_num_unary_op(pos, expr.operation, *value))
                .collect::<CompileResult<Vec<_>>>()
        });
        match results {
            Ok(results) => Ok(self.squash_values(results)),
            Err(err) => Err(err),
        }
    }

    fn lower_tuple_expr(&mut self, expr: &'a ast::TupleExpression) -> LowerResult {
        let values: CompileResult<Vec<_>> = expr.expressions
            .iter()
            .map(|expression| self.lower_expression(expression))
            .collect();

        match values {
            Ok(values) => Ok(self.squash_values(values)),
            Err(err) => Err(err),
        }
    }

    fn lower_variable_expr(
        &mut self,
        pos: &ast::SourceRange,
        expr: &'a ast::VariableExpression,
    ) -> LowerResult {
        match self.var_indexes.get::<str>(&expr.name) {
            Some(index) => Ok(*index),
            None => Err(CompileError::unknown_variable(expr.name.clone(), *pos)),
        }
    }

    fn get_control_index(&mut self, expr: &'a ast::ControlExpression) -> usize {
        let control_key = (expr.name.as_ref(), ast::ControlType::from(expr.field));
        match self.control_indexes.get(&control_key).cloned() {
            Some(index) => index,
            None => {
                let result_index = self.block.controls.len();
                self.block.controls.push(mir::block::Control::new(
                    expr.name.clone(),
                    control_key.1,
                    false,
                    false,
                ));
                self.control_indexes.insert(control_key, result_index);
                result_index
            }
        }
    }

    fn add_statement(&mut self, statement: mir::block::Statement) -> usize {
        self.block.statements.push(statement);
        self.block.statements.len() - 1
    }

    fn add_num_convert(
        &mut self,
        pos: &ast::SourceRange,
        target_form: ast::FormType,
        input: usize,
    ) -> LowerResult {
        let new_statement = if let Some(const_input) = self.get_num_constant(pos, input) {
            match const_input {
                Ok(const_num) => mir::block::Statement::new_const_num(
                    constant_propagate::const_convert(const_num, &target_form),
                ),
                Err(err) => return Err(err),
            }
        } else if let Some(err) = self.check_statement_type(pos, mir::VarType::Num, input) {
            return Err(err);
        } else {
            mir::block::Statement::NumConvert { target_form, input }
        };

        Ok(self.add_statement(new_statement))
    }

    fn add_num_cast(
        &mut self,
        pos: &ast::SourceRange,
        target_form: ast::FormType,
        input: usize,
    ) -> LowerResult {
        let new_statement = if let Some(const_input) = self.get_num_constant(pos, input) {
            match const_input {
                Ok(const_num) => mir::block::Statement::new_const_num(
                    constant_propagate::const_cast(const_num, target_form),
                ),
                Err(err) => return Err(err),
            }
        } else if let Some(err) = self.check_statement_type(pos, mir::VarType::Num, input) {
            return Err(err);
        } else {
            mir::block::Statement::NumCast { target_form, input }
        };

        Ok(self.add_statement(new_statement))
    }

    fn add_num_unary_op(
        &mut self,
        pos: &ast::SourceRange,
        op: ast::UnaryOperation,
        input: usize,
    ) -> LowerResult {
        let new_statement = if let Some(const_input) = self.get_num_constant(pos, input) {
            match const_input {
                Ok(const_num) => mir::block::Statement::new_const_num(
                    constant_propagate::const_unary_op(const_num, op),
                ),
                Err(err) => return Err(err),
            }
        } else if let Some(err) = self.check_statement_type(pos, mir::VarType::Num, input) {
            return Err(err);
        } else {
            mir::block::Statement::NumUnaryOp { op, input }
        };

        Ok(self.add_statement(new_statement))
    }

    fn add_num_math_op(
        &mut self,
        pos: &ast::SourceRange,
        op: ast::OperatorType,
        lhs: usize,
        rhs: usize,
    ) -> LowerResult {
        let new_statement = {
            let const_vals = if let Some(const_lhs) = self.get_num_constant(pos, lhs) {
                if let Some(const_rhs) = self.get_num_constant(pos, rhs) {
                    Some((const_lhs, const_rhs))
                } else {
                    None
                }
            } else {
                None
            };

            if let Some((const_lhs, const_rhs)) = const_vals {
                let lhs_num = match const_lhs {
                    Ok(const_num) => const_num.clone(),
                    Err(err) => return Err(err),
                };
                let rhs_num = match const_rhs {
                    Ok(const_num) => const_num.clone(),
                    Err(err) => return Err(err),
                };
                mir::block::Statement::new_const_num(constant_propagate::const_math_op(
                    &lhs_num, &rhs_num, op,
                ))
            } else if let Some(err) = self.check_statement_type(pos, mir::VarType::Num, lhs) {
                return Err(err);
            } else if let Some(err) = self.check_statement_type(pos, mir::VarType::Num, rhs) {
                return Err(err);
            } else {
                mir::block::Statement::NumMathOp { op, lhs, rhs }
            }
        };

        Ok(self.add_statement(new_statement))
    }

    fn add_extract_op(
        &mut self,
        pos: &ast::SourceRange,
        tuple: usize,
        index: usize,
    ) -> LowerResult {
        let new_statement = if let Some(const_input) = self.get_tuple_constant(pos, tuple) {
            match const_input {
                Ok(const_tuple) => match constant_propagate::const_extract(const_tuple, index, pos)
                {
                    Ok(constant) => mir::block::Statement::Constant(constant.clone()),
                    Err(err) => return Err(err),
                },
                Err(err) => return Err(err),
            }
        } else {
            match mir::VarType::of_statement(&self.block, tuple) {
                mir::VarType::Tuple(ref item_types) if index >= item_types.len() => {
                    return Err(CompileError::access_out_of_bounds(
                        item_types.len(),
                        index,
                        *pos,
                    ))
                }
                mir::VarType::Tuple(_) => mir::block::Statement::Extract { tuple, index },
                var_type => {
                    return Err(CompileError::mismatched_type(
                        mir::VarType::Tuple(Vec::new()),
                        var_type,
                        *pos,
                    ))
                }
            }
        };

        Ok(self.add_statement(new_statement))
    }

    fn add_combine_op(&mut self, indexes: Vec<usize>) -> usize {
        let const_inputs: Option<Vec<_>> = indexes
            .iter()
            .map(|index| self.get_constant(*index).cloned())
            .collect();
        if let Some(const_vals) = const_inputs {
            self.add_statement(mir::block::Statement::new_const_tuple(
                constant_propagate::const_combine(const_vals),
            ))
        } else {
            self.block
                .statements
                .push(mir::block::Statement::Combine { indexes });
            self.block.statements.len() - 1
        }
    }

    fn add_call_func(&mut self, function: mir::block::Function, args: Vec<usize>) -> usize {
        unimplemented!();
    }

    fn add_store_control(
        &mut self,
        pos: &ast::SourceRange,
        control: usize,
        field: ast::ControlField,
        value: usize,
    ) -> LowerResult {
        if let Some(err) =
            self.check_statement_type(pos, mir::VarType::of_control_field(&field), value)
        {
            return Err(err);
        }

        self.block
            .statements
            .push(mir::block::Statement::StoreControl {
                control,
                field,
                value,
            });
        Ok(self.block.statements.len() - 1)
    }

    fn add_load_control(&mut self, control: usize, field: ast::ControlField) -> usize {
        self.block
            .statements
            .push(mir::block::Statement::LoadControl { control, field });
        self.block.statements.len() - 1
    }

    fn get_constant(&self, value: usize) -> Option<&mir::block::ConstantValue> {
        match &self.block.statements[value] {
            mir::block::Statement::Constant(ref val) => Some(val),
            _ => None,
        }
    }

    fn get_num_constant(
        &self,
        pos: &ast::SourceRange,
        value: usize,
    ) -> Option<CompileResult<&mir::block::ConstantNum>> {
        if let Some(const_val) = self.get_constant(value) {
            Some(if let Some(num_val) = const_val.as_num() {
                Ok(num_val)
            } else {
                Err(CompileError::mismatched_type(
                    mir::VarType::Num,
                    mir::VarType::of_constant(const_val),
                    *pos,
                ))
            })
        } else {
            None
        }
    }

    fn get_tuple_constant(
        &self,
        pos: &ast::SourceRange,
        value: usize,
    ) -> Option<CompileResult<&mir::block::ConstantTuple>> {
        if let Some(const_val) = self.get_constant(value) {
            Some(if let Some(tuple_val) = const_val.as_tuple() {
                Ok(tuple_val)
            } else {
                Err(CompileError::mismatched_type(
                    mir::VarType::Tuple(Vec::new()),
                    mir::VarType::of_constant(const_val),
                    *pos,
                ))
            })
        } else {
            None
        }
    }

    fn check_statement_type(
        &self,
        pos: &ast::SourceRange,
        expected_type: mir::VarType,
        value: usize,
    ) -> Option<CompileError> {
        self.check_type(
            pos,
            expected_type,
            mir::VarType::of_statement(&self.block, value),
        )
    }

    fn check_type(
        &self,
        pos: &ast::SourceRange,
        expected_type: mir::VarType,
        actual_type: mir::VarType,
    ) -> Option<CompileError> {
        if expected_type != actual_type {
            Some(CompileError::mismatched_type(
                expected_type,
                actual_type,
                pos.clone(),
            ))
        } else {
            None
        }
    }
}
