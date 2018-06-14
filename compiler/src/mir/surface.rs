use mir::pool_id::PoolId;

pub type SurfaceId = PoolId<Surface>;

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct Surface {
    pub id: SurfaceId,
}

impl Surface {}
