#[derive(Debug, Clone)]
pub struct TargetProperties {
    pub include_ui: bool,
    pub min_size: bool,
}

impl TargetProperties {
    pub fn new(include_ui: bool, min_size: bool) -> Self {
        TargetProperties {
            include_ui,
            min_size,
        }
    }
}
