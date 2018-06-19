use mir::MIRContext;
use std::cmp::Ordering;
use std::hash::{Hash, Hasher};
use std::marker::PhantomData;

#[derive(Debug, Clone)]
pub struct PoolId<T> {
    pub id: u64,
    pub debug_name: String,
    phantom: PhantomData<T>,
}

impl<T> PartialEq for PoolId<T> {
    fn eq(&self, other: &PoolId<T>) -> bool {
        self.id == other.id
    }
}

impl<T> Eq for PoolId<T> {}

impl<T> Ord for PoolId<T> {
    fn cmp(&self, other: &PoolId<T>) -> Ordering {
        self.id.cmp(&other.id)
    }
}

impl<T> PartialOrd for PoolId<T> {
    fn partial_cmp(&self, other: &PoolId<T>) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl<T> Hash for PoolId<T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.id.hash(state);
    }
}

impl<T> PoolId<T> {
    pub fn new(debug_name: String, allocator: &mut IdAllocator) -> PoolId<T> {
        PoolId {
            id: allocator.alloc_id(),
            debug_name,
            phantom: PhantomData,
        }
    }

    pub fn new_with_id(debug_name: String, id: u64) -> PoolId<T> {
        PoolId {
            id,
            debug_name,
            phantom: PhantomData,
        }
    }
}

pub trait IdAllocator {
    fn alloc_id(&mut self) -> u64;
}
