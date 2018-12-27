use crate::util::feature_level::FeatureLevel;
use std::path::PathBuf;

#[derive(Debug, Clone, Copy)]
pub struct AudioConfig {
    pub sample_rate: f64,
    pub bpm: f64,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum TargetPlatform {
    WindowsMsvc,
    WindowsGnu,
    Mac,
    Linux,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum TargetInstructionSet {
    I686,
    X64,
}

#[derive(Debug, Clone, Copy)]
pub struct TargetConfig {
    pub platform: TargetPlatform,
    pub instruction_set: TargetInstructionSet,
    pub feature_level: FeatureLevel,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum OptimizationLevel {
    None,
    Low,
    Medium,
    High,
    MinSize,
    AggressiveSize,
}

#[derive(Debug, Clone)]
pub struct CodeConfig {
    pub optimization_level: OptimizationLevel,
    pub instrument_prefix: String,
    pub include_instrument: bool,
    pub include_library: bool,
}

#[derive(Debug, Clone)]
pub struct ObjectOutputConfig {
    pub location: PathBuf,
}

#[derive(Debug, Clone)]
pub struct MetaOutputConfig {
    pub location: PathBuf,
    pub portal_names: Vec<String>,
}

#[derive(Debug, Clone)]
pub struct ExportConfig {
    pub audio: AudioConfig,
    pub target: TargetConfig,
    pub code: CodeConfig,
    pub object: Option<ObjectOutputConfig>,
    pub meta: Option<MetaOutputConfig>,
}
