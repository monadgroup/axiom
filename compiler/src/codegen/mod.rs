pub mod block;
mod block_context;
mod builder_context;
mod control_context;
pub mod controls;
pub mod converters;
pub mod data_analyzer;
pub mod functions;
pub mod globals;
pub mod intrinsics;
pub mod statements;
mod target_properties;
pub mod util;
pub mod values;

pub use self::block_context::BlockContext;
pub use self::builder_context::{build_context_function, BuilderContext};
pub use self::control_context::{ControlContext, ControlUiContext};
pub use self::target_properties::TargetProperties;

pub enum LifecycleFunc {
    Construct,
    Update,
    Destruct,
}
