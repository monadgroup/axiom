use super::export_config::{CodeConfig, MetaFormat, MetaOutputConfig};
use lazy_static::lazy_static;
use regex::Regex;
use std::borrow::Cow;
use std::collections::HashMap;
use std::{fmt, path};
use tasty::{process_template, Error};

lazy_static! {
    static ref NON_SAFE_CHARACTERS_REGEX: Regex = Regex::new(r"_?[^0-9A-Za-z_]+_?").unwrap();
}

pub struct ModuleMetadata {
    pub init_func_name: String,
    pub cleanup_func_name: String,
    pub generate_func_name: String,
    pub portal_func_name: String,
}

fn determine_c_file_name(output_path: &path::Path) -> Option<String> {
    let file_name = output_path.file_name()?.to_str()?;
    let safe_file_name = NON_SAFE_CHARACTERS_REGEX.replace(file_name, "_");
    Some(safe_file_name.to_uppercase())
}

pub fn build_meta_output(
    f: &mut dyn fmt::Write,
    code_config: &CodeConfig,
    meta_config: &MetaOutputConfig,
    module_data: &ModuleMetadata,
) -> fmt::Result {
    let c_file_name = determine_c_file_name(&meta_config.location).unwrap();
    let def_prefix = code_config.instrument_prefix.to_uppercase();
    let portal_count = meta_config.portal_names.len().to_string();
    let template_str = match meta_config.format {
        MetaFormat::CHeader => include_str!("header_template.h.tasty"),
        MetaFormat::RustModule => include_str!("rust_module_template.rs.tasty"),
        MetaFormat::Json => include_str!("json_template.json.tasty"),
    };
    let mut context: HashMap<_, &str> = HashMap::new();
    context.insert(Cow::Borrowed("C_FILE_NAME"), &c_file_name);
    context.insert(Cow::Borrowed("FUNC_PREFIX"), &code_config.instrument_prefix);
    context.insert(Cow::Borrowed("DEF_PREFIX"), &def_prefix);
    context.insert(Cow::Borrowed("PORTAL_COUNT"), &portal_count);
    for (portal_index, portal_name) in meta_config.portal_names.iter().enumerate() {
        context.insert(
            Cow::Owned(format!("PORTAL_NAME_{}", portal_index)),
            portal_name,
        );
    }
    context.insert(Cow::Borrowed("INIT_FUNC_NAME"), &module_data.init_func_name);
    context.insert(
        Cow::Borrowed("CLEANUP_FUNC_NAME"),
        &module_data.cleanup_func_name,
    );
    context.insert(
        Cow::Borrowed("GENERATE_FUNC_NAME"),
        &module_data.generate_func_name,
    );
    context.insert(
        Cow::Borrowed("PORTAL_FUNC_NAME"),
        &module_data.portal_func_name,
    );

    match process_template(f, template_str, &context) {
        Err(Error::Writer(err)) => Err(err),
        other => Ok(other.unwrap()),
    }
}
