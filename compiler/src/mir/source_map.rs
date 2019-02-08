use std::collections::HashMap;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum InternalNodeRef {
    Direct(usize),
    Surface(usize, usize),
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
enum ExternalNodeRef {
    Direct(usize),
    Deleted(usize),
}

impl InternalNodeRef {
    pub fn node(&self) -> usize {
        match self {
            InternalNodeRef::Direct(node) => *node,
            InternalNodeRef::Surface(node, _) => *node,
        }
    }

    pub fn with_node(self, node: usize) -> InternalNodeRef {
        match self {
            InternalNodeRef::Direct(_) => InternalNodeRef::Direct(node),
            InternalNodeRef::Surface(_, sub) => InternalNodeRef::Surface(node, sub),
        }
    }
}

#[derive(Debug, Clone, Default)]
pub struct SourceMap {
    map: HashMap<ExternalNodeRef, InternalNodeRef>,
    deleted_index: usize,
}

impl SourceMap {
    pub fn new() -> Self {
        SourceMap {
            map: HashMap::new(),
            deleted_index: 0,
        }
    }

    pub fn map_to_internal(&self, external: usize) -> InternalNodeRef {
        if let Some(internal) = self.map.get(&ExternalNodeRef::Direct(external)) {
            *internal
        } else {
            InternalNodeRef::Direct(external)
        }
    }

    pub fn map_to_external(&self, internal: usize) -> Vec<usize> {
        let mut mapped_items: Vec<_> = self
            .map
            .iter()
            .filter(move |&(_, internal_ref)| internal_ref.node() == internal)
            .map(|(&external, _)| external)
            .collect();
        if mapped_items.is_empty() {
            mapped_items.push(ExternalNodeRef::Direct(internal));
        }
        mapped_items
            .into_iter()
            .filter_map(|val| match val {
                ExternalNodeRef::Direct(v) => Some(v),
                ExternalNodeRef::Deleted(_) => None,
            })
            .collect()
    }

    pub fn move_to(&mut self, movements: impl IntoIterator<Item = (usize, usize)>) {
        let mut new_map = self.map.clone();

        for (before_internal, after_internal) in movements {
            let externals = self.map_to_external(before_internal);
            for external in externals.into_iter() {
                let old_internal = self.map_to_internal(external);
                let new_internal = old_internal.with_node(after_internal);
                SourceMap::insert(
                    &mut new_map,
                    ExternalNodeRef::Direct(external),
                    new_internal,
                );
            }
        }

        self.map = new_map;
    }

    pub fn move_into(
        &mut self,
        before_internal: usize,
        internal: InternalNodeRef,
    ) -> Vec<InternalNodeRef> {
        let externals = self.map_to_external(before_internal);
        let moved_vals = externals
            .into_iter()
            .filter_map(|external| {
                let removed = self.map.remove(&ExternalNodeRef::Direct(external));

                // review: does this work in all cases? is there anything else we need to do here?
                SourceMap::insert(&mut self.map, ExternalNodeRef::Direct(external), internal);
                removed
            })
            .collect();

        let next_deleted_index = self.deleted_index;
        self.deleted_index += 1;
        SourceMap::insert(
            &mut self.map,
            ExternalNodeRef::Deleted(next_deleted_index),
            InternalNodeRef::Direct(before_internal),
        );
        moved_vals
    }

    fn insert(
        map: &mut HashMap<ExternalNodeRef, InternalNodeRef>,
        external: ExternalNodeRef,
        internal: InternalNodeRef,
    ) {
        // if the map is a noop, remove it instead
        match internal {
            InternalNodeRef::Direct(node) if ExternalNodeRef::Direct(node) == external => {
                map.remove(&external);
            }
            _ => {
                map.insert(external, internal);
            }
        }
    }
}
