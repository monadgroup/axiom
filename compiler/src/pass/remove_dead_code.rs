use mir;

pub fn remove_dead_code(block: &mut mir::Block) {
    DeadCodeRemover::new(block).remove_dead_code()
}

struct DeadCodeRemover<'a> {
    pub block: &'a mut mir::Block,
}

impl<'a> DeadCodeRemover<'a> {
    pub fn new(block: &'a mut mir::Block) -> DeadCodeRemover<'a> {
        DeadCodeRemover { block }
    }

    pub fn remove_dead_code(&mut self) {
        // keep looping until all dead code has been removed
        loop {
            let counts = self.build_statement_counts();
            match self.remove_dead_statements(&counts) {
                Some(mapping) => self.remap_refs(&mapping),
                None => break,
            }
        }
    }

    fn build_statement_counts(&mut self) -> Vec<usize> {
        let mut result = vec![0; self.block.statements.len()];
        for statement in &mut self.block.statements {
            for ref_index in DeadCodeRemover::get_statement_refs(statement) {
                result[*ref_index] += 1
            }
        }
        result
    }

    fn remove_dead_statements(&mut self, statement_counts: &[usize]) -> Option<Vec<usize>> {
        let mut iter_index: usize = 0;
        let mut offset: usize = 0;
        let mut remap = vec![0; self.block.statements.len()];
        self.block.statements.retain(|statement| {
            let i = iter_index;
            iter_index += 1;

            remap[i] = i - offset;

            // the statement can't be removed if it has side effects or has >0 references
            if statement.has_side_effect() || statement_counts[i] > 0 {
                true
            } else {
                offset += 1;
                false
            }
        });

        if offset == 0 {
            None
        } else {
            Some(remap)
        }
    }

    fn remap_refs(&mut self, mappings: &[usize]) {
        for statement in &mut self.block.statements {
            for ref_target in DeadCodeRemover::get_statement_refs(statement) {
                *ref_target = mappings[*ref_target]
            }
        }
    }

    fn get_statement_refs(statement: &mut mir::block::Statement) -> Vec<&mut usize> {
        match statement {
            mir::block::Statement::Constant(_) => vec![],
            mir::block::Statement::NumConvert { ref mut input, .. } => vec![input],
            mir::block::Statement::NumCast { ref mut input, .. } => vec![input],
            mir::block::Statement::NumUnaryOp { ref mut input, .. } => vec![input],
            mir::block::Statement::NumMathOp {
                ref mut lhs,
                ref mut rhs,
                ..
            } => vec![lhs, rhs],
            mir::block::Statement::Extract { ref mut tuple, .. } => vec![tuple],
            mir::block::Statement::Combine {
                ref mut indexes, ..
            } => indexes.iter_mut().collect(),
            mir::block::Statement::CallFunc { ref mut args, .. } => args.iter_mut().collect(),
            mir::block::Statement::StoreControl { ref mut value, .. } => vec![value],
            mir::block::Statement::LoadControl { .. } => vec![],
        }
    }
}
