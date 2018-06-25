pub mod block;
mod builder_context;
pub mod controls;
pub mod converters;
pub mod data_analyzer;
pub mod functions;
pub mod globals;
pub mod intrinsics;
mod target_properties;
pub mod util;
pub mod values;

pub use self::builder_context::{build_context_function, BuilderContext};
pub use self::target_properties::TargetProperties;

pub enum LifecycleFunc {
    Construct,
    Update,
    Destruct,
}
