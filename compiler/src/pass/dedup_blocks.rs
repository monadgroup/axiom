use crate::mir;
use std::collections::hash_map::Entry;
use std::collections::HashMap;
use std::hash;

// Compares between blocks by looking at if they're functionally equivalent (that is, with different
// IDs but the same behaviour). Concretely, this means the control types must be identical, and the
// statements must be identical.
struct FunctionallyEquivalentBlock<'block>(&'block mir::Block);

impl PartialEq for FunctionallyEquivalentBlock<'_> {
    fn eq(&self, other: &FunctionallyEquivalentBlock) -> bool {
        // first check control types - early exit if lengths are different
        if self.0.controls.len() != other.0.controls.len() {
            return false;
        }

        for (self_control, other_control) in self.0.controls.iter().zip(other.0.controls.iter()) {
            if self_control.control_type != other_control.control_type {
                return false;
            }
        }

        // if the statements are identical, the blocks are identical
        self.0.statements == other.0.statements
    }
}

impl Eq for FunctionallyEquivalentBlock<'_> {}

impl hash::Hash for FunctionallyEquivalentBlock<'_> {
    fn hash<H: hash::Hasher>(&self, state: &mut H) {
        self.0.controls.len().hash(state);
        for control in &self.0.controls {
            control.control_type.hash(state);
        }
        self.0.statements.hash(state);
    }
}

/// There are three steps to deduplicate blocks in a project:
///  - Build a map of functionally equivalent blocks to the ID of the first encountered block with that equivalence
///  - For each block, find all referenced surfaces and swap the reference with the base block (from the map)
///  - Remove the duplicate blocks from the MIR
pub fn deduplicate_blocks<'surface>(
    blocks: &mut HashMap<mir::BlockRef, mir::Block>,
    surfaces: impl IntoIterator<Item = &'surface mut mir::Surface>,
) {
    let (equivalent_refs, delete_blocks) = build_block_equivalence_map(blocks.values());
    fix_block_references(surfaces, equivalent_refs);
    remove_old_blocks(blocks, &delete_blocks);
}

fn build_block_equivalence_map<'block>(
    blocks: impl IntoIterator<Item = &'block mir::Block>,
) -> (HashMap<mir::BlockRef, mir::BlockRef>, Vec<mir::BlockRef>) {
    let mut hashed_equivalence_map = HashMap::new();
    let mut id_equivalence_map = HashMap::new();
    let mut can_delete_blocks = Vec::new();

    for block in blocks {
        match hashed_equivalence_map.entry(FunctionallyEquivalentBlock(block)) {
            Entry::Vacant(v) => {
                v.insert(block.id.id);
            }
            Entry::Occupied(o) => {
                id_equivalence_map.insert(block.id.id, *o.get());
                can_delete_blocks.push(block.id.id);
            }
        }
    }

    (id_equivalence_map, can_delete_blocks)
}

fn fix_block_references<'surface>(
    surfaces: impl IntoIterator<Item = &'surface mut mir::Surface>,
    equivalence_map: HashMap<mir::BlockRef, mir::BlockRef>,
) {
    for surface in surfaces {
        for node in &mut surface.nodes {
            if let mir::NodeData::Custom(block_ref) = &mut node.data {
                if let Some(equivalent_id) = equivalence_map.get(block_ref) {
                    *block_ref = *equivalent_id;
                }
            }
        }
    }
}

fn remove_old_blocks(
    all_blocks: &mut HashMap<mir::BlockRef, mir::Block>,
    delete_blocks: &[mir::BlockRef],
) {
    for delete_block in delete_blocks {
        all_blocks.remove(delete_block);
    }
}
