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

pub fn get_target_feature_string(feature_level: FeatureLevel) -> String {
    // we need SSE4.1 at a minimum, so dynamically enable SSE4.2, AVX, and AVX2 if we can
    let mut base_features = "+x87,+mmx,+sse,+sse2,+sse3,+ssse3,+sse4.1".to_string();

    if feature_level >= FeatureLevel::SSE42 {
        base_features.push_str(",+sse4.2");
    }
    if feature_level >= FeatureLevel::AVX {
        base_features.push_str(",+avx");
    }
    if feature_level >= FeatureLevel::AVX2 {
        base_features.push_str(",+avx2");
    }

    base_features
}
