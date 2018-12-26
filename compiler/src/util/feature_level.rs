use lazy_static::lazy_static;

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(u8)]
pub enum FeatureLevel {
    SSE41,
    SSE42,
    AVX,
    AVX2,
}

fn get_feature_level() -> FeatureLevel {
    if is_x86_feature_detected!("avx2") {
        FeatureLevel::AVX2
    } else if is_x86_feature_detected!("avx") {
        FeatureLevel::AVX
    } else if is_x86_feature_detected!("sse4.2") {
        FeatureLevel::SSE42
    } else if is_x86_feature_detected!("sse4.1") {
        FeatureLevel::SSE41
    } else {
        panic!("Axiom requires a CPU that supports at least SSE4.1")
    }
}

lazy_static! {
    pub static ref FEATURE_LEVEL: FeatureLevel = get_feature_level();
}
