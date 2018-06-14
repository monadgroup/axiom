use mir::pool_id::PoolId;

pub type BlockId = PoolId<Block>;

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Block {
    id: BlockId,
}

impl Block {}
