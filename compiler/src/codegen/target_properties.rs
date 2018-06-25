#[derive(Debug, Clone)]
pub struct TargetProperties {
    pub include_ui: bool,
}

impl TargetProperties {
    pub fn new(include_ui: bool) -> Self {
        TargetProperties { include_ui }
    }
}
