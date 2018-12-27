use core::ffi::c_void;

pub const {{DEF_PREFIX}}SAMPLERATE: f64 = {{SAMPLERATE}};
pub const {{DEF_PREFIX}}BPM: f64 = {{BPM}};

{%LOOP PORTAL_COUNT%}
pub const {{PORTAL_NAME_{%INDEX%}}}: usize = {%INDEX%};
{%END%}

extern "cdecl" {
fn {{FUNC_PREFIX}}init();
fn {{FUNC_PREFIX}}cleanup();
fn {{FUNC_PREFIX}}generate();

fn {{FUNC_PREFIX}}get_portal(id: u32): *mut c_void;
}
