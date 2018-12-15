use std::fmt;
use std::marker::PhantomData;

pub type PoolRef = u64;

#[derive(Clone)]
pub struct PoolId<T> {
    pub id: u64,
    pub debug_name: String,
    phantom: PhantomData<T>,
}

impl<T> fmt::Debug for PoolId<T> {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        write!(f, "{} {}", self.id, self.debug_name)
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

impl<T> PartialEq for PoolId<T> {
    fn eq(&self, other: &PoolId<T>) -> bool {
        self.id == other.id
    }
}

impl<T> Eq for PoolId<T> {}

pub trait IdAllocator {
    fn alloc_id(&mut self) -> u64;
}

#[derive(Debug, Clone, Copy)]
pub struct IncrementalIdAllocator {
    next_id: u64,
}

impl IncrementalIdAllocator {
    pub fn new(start_index: u64) -> Self {
        IncrementalIdAllocator {
            next_id: start_index,
        }
    }

    pub fn reserve(&mut self, index: u64) {
        self.next_id = self.next_id.max(index + 1)
    }
}

impl IdAllocator for IncrementalIdAllocator {
    fn alloc_id(&mut self) -> u64 {
        let take_id = self.next_id;
        self.next_id += 1;
        take_id
    }
}
