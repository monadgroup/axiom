mod build_instrument_module;
mod build_meta_output;
pub mod export_config;

use self::build_instrument_module::build_instrument_module;
use self::build_meta_output::{build_meta_output, ModuleMetadata};
use self::export_config::{
    AudioConfig, CodeConfig, ExportConfig, MetaOutputConfig, ObjectFormat, ObjectOutputConfig,
    TargetConfig, TargetInstructionSet, TargetPlatform,
};
use super::Transaction;
use crate::codegen::{
    globals, runtime_lib, util, ModuleFunctionIterator, ModuleGlobalIterator, Optimizer,
    TargetProperties,
};
use crate::util::feature_level::get_target_feature_string;
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::targets::{CodeModel, FileType, RelocMode, Target};
use inkwell::types::VectorType;
use std::fs;

fn export_meta(
    config: &MetaOutputConfig,
    audio_conf: &AudioConfig,
    code_conf: &CodeConfig,
    module_meta: &ModuleMetadata,
) {
    let mut meta_output = String::new();
    build_meta_output(&mut meta_output, audio_conf, code_conf, config, module_meta).unwrap();

    fs::write(&config.location, &meta_output).unwrap(); // todo: don't unwrap
}

fn get_target_cpu(instruction_set: TargetInstructionSet) -> &'static str {
    match instruction_set {
        TargetInstructionSet::I686 => "i686",
        TargetInstructionSet::X64 => "x86-64",
    }
}

fn get_target_triple(target_conf: &TargetConfig) -> String {
    let cpu_specifier = match target_conf.instruction_set {
        TargetInstructionSet::I686 => "i686",
        TargetInstructionSet::X64 => "x86_64",
    };
    let platform_specifier = match target_conf.platform {
        TargetPlatform::WindowsMsvc => "pc-windows-msvc",
        TargetPlatform::WindowsGnu => "w64-windows-gnu",
        TargetPlatform::Mac => "apple-darwin-macho",
        TargetPlatform::Linux => "unknown-linux-gnu",
    };

    cpu_specifier.to_string() + "-" + platform_specifier
}

fn export_object(
    config: &ObjectOutputConfig,
    audio_conf: &AudioConfig,
    target_conf: &TargetConfig,
    code_conf: &CodeConfig,
    module_meta: &ModuleMetadata,
    transaction: Transaction,
) {
    let target_triple = get_target_triple(target_conf);
    let target_cpu = get_target_cpu(target_conf.instruction_set);
    let target_features = get_target_feature_string(target_conf.feature_level);

    let target = Target::from_triple(&target_triple).unwrap();

    let machine = target
        .create_target_machine(
            &target_triple,
            target_cpu,
            &target_features,
            code_conf.optimization_level.into_specification().llvm_level,
            RelocMode::Default,
            CodeModel::Default,
        )
        .unwrap();
    let target_properties = TargetProperties::new(false, code_conf.optimization_level, machine);

    let context = Context::create();
    let output_module = target_properties.create_module(
        &context,
        config.location.file_name().unwrap().to_str().unwrap(),
    );

    if code_conf.include_library {
        // build constant globals
        let sample_rate_global = globals::get_sample_rate(&output_module);
        sample_rate_global.set_constant(true);
        sample_rate_global.set_initializer(&util::get_vec_spread(&context, audio_conf.sample_rate));

        let bpm_global = globals::get_bpm(&output_module);
        bpm_global.set_constant(true);
        bpm_global.set_initializer(&util::get_vec_spread(&context, audio_conf.bpm));
        globals::get_rand_seed(&output_module).set_initializer(&VectorType::const_vector(&[
            &context.i64_type().const_int(1, false),
            &context.i64_type().const_int(31337, false),
        ]));

        // build the library
        runtime_lib::codegen_lib(&output_module, &target_properties);
    }
    if code_conf.include_instrument {
        build_instrument_module(
            &context,
            &output_module,
            &target_properties,
            transaction,
            module_meta,
        );

        hide_internal_symbols(&output_module);
    }

    // optimize the module
    let optimizer = Optimizer::new(&target_properties);
    optimizer.optimize_module(&output_module);

    // write the output to the specified file
    match config.format {
        ObjectFormat::Object => {
            let mem_buf = target_properties
                .machine
                .write_to_memory_buffer(&output_module, FileType::Object)
                .unwrap();

            fs::write(&config.location, mem_buf.as_slice()).unwrap();
        }
        ObjectFormat::Bitcode => {
            if !output_module.write_bitcode_to_path(&config.location) {
                panic!();
            }
        }
        ObjectFormat::IR => {
            fs::write(
                &config.location,
                output_module.print_to_string().to_str().unwrap(),
            )
            .unwrap();
        }
        ObjectFormat::AssemblyListing => {
            let mem_buf = target_properties
                .machine
                .write_to_memory_buffer(&output_module, FileType::Assembly)
                .unwrap();

            fs::write(&config.location, mem_buf.as_slice()).unwrap();
        }
    }
}

fn hide_internal_symbols(module: &Module) {
    let func_iterator = ModuleFunctionIterator::new(module);
    for func in func_iterator {
        if func.get_name().to_str().unwrap().starts_with("maxim.") && !func.is_declaration() {
            func.set_linkage(Linkage::PrivateLinkage);
        }
    }

    let global_iterator = ModuleGlobalIterator::new(module);
    for global in global_iterator {
        if global.get_name().to_str().unwrap().starts_with("maxim.") && !global.is_declaration() {
            global.set_linkage(Linkage::PrivateLinkage);
        }
    }
}

pub fn export(config: &ExportConfig, transaction: Transaction) {
    // Generate the module metadata
    let module_meta = ModuleMetadata {
        init_func_name: config.code.instrument_prefix.clone() + "init",
        cleanup_func_name: config.code.instrument_prefix.clone() + "cleanup",
        generate_func_name: config.code.instrument_prefix.clone() + "generate",
        portal_func_name: config.code.instrument_prefix.clone() + "portal",
    };

    // Export the requested data
    if let Some(meta_config) = &config.meta {
        export_meta(meta_config, &config.audio, &config.code, &module_meta);
    }
    if let Some(object_config) = &config.object {
        export_object(
            object_config,
            &config.audio,
            &config.target,
            &config.code,
            &module_meta,
            transaction,
        );
    }
}
