mod amplitude_converter;
mod beats_converter;
mod control_converter;
mod db_converter;
mod frequency_converter;
mod note_converter;
mod oscillator_converter;
mod q_converter;
mod samples_converter;
mod seconds_converter;

use ast::FormType;
use codegen::util;
use codegen::values::NumValue;
use inkwell::basic_block::BasicBlock;
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::values::{FunctionValue, IntValue, StructValue, VectorValue};
use inkwell::AddressSpace;

pub type ConvertGeneratorCb = Fn(&Context, &Module, &mut Builder, VectorValue) -> VectorValue;

pub trait ConvertGenerator {
    fn generate(&mut self, form: FormType, cb: &ConvertGeneratorCb);
}

struct PrivateGenerator<'a> {
    func: &'a FunctionValue,
    input_vec: &'a VectorValue,
    result_num: &'a NumValue,
    end_block: &'a BasicBlock,
    context: &'a Context,
    module: &'a Module,
    branches: Vec<(IntValue, BasicBlock)>,
}

impl<'a> ConvertGenerator for PrivateGenerator<'a> {
    fn generate(&mut self, form: FormType, cb: &ConvertGeneratorCb) {
        let new_block = self.context
            .append_basic_block(self.func, &format!("to.{}", form));
        let mut builder = self.context.create_builder();
        builder.set_fast_math_all();
        builder.position_at_end(&new_block);

        let result_vec = cb(self.context, self.module, &mut builder, *self.input_vec);
        self.result_num.set_vec(&mut builder, &result_vec);
        builder.build_unconditional_branch(self.end_block);

        let form_num = self.context.i8_type().const_int(form as u64, false);
        self.branches.push((form_num, new_block));
    }
}

pub fn get_convert_func(module: &Module, target_form: FormType) -> FunctionValue {
    let func_name = format!("maxim.converter.{}", target_form);
    util::get_or_create_func(module, &func_name, true, &|| {
        let num_type = NumValue::get_type(&module.get_context());
        (
            Linkage::ExternalLinkage,
            num_type.fn_type(&[&num_type.ptr_type(AddressSpace::Generic)], false),
        )
    })
}

pub fn build_convert_func(
    module: &Module,
    target_form: FormType,
    build_func: &Fn(&mut ConvertGenerator),
) {
    let context = &module.get_context();
    let func = get_convert_func(module, target_form);
    let block = context.append_basic_block(&func, "main");
    let mut builder = context.create_builder();
    builder.set_fast_math_all();
    builder.position_at_end(&block);

    let input_num = NumValue::new(func.get_nth_param(0).unwrap().into_pointer_value());
    let result_num = NumValue::new_undef(context, &mut builder);
    let input_form = input_num.get_form(&mut builder);
    let input_vec = input_num.get_vec(&mut builder);

    result_num.set_form(
        &mut builder,
        &context.i8_type().const_int(target_form as u64, false),
    );

    // build up each switch block
    let end_block = context.append_basic_block(&func, "end");

    let default_block = context.append_basic_block(&func, "default");
    builder.position_at_end(&default_block);
    result_num.set_vec(&mut builder, &input_vec);
    builder.build_unconditional_branch(&end_block);

    let mut converter = PrivateGenerator {
        func: &func,
        input_vec: &input_vec,
        result_num: &result_num,
        end_block: &end_block,
        context,
        module,
        branches: Vec::new(),
    };
    build_func(&mut converter);

    let switch_cases: Vec<_> = converter
        .branches
        .iter()
        .map(|&(ref form_type, ref block)| (form_type, block))
        .collect();
    builder.position_at_end(&block);
    builder.build_switch(&input_form, &default_block, &switch_cases);

    builder.position_at_end(&end_block);
    let return_val = result_num.load(&mut builder);
    builder.build_return(Some(&return_val));
}

pub fn build_funcs(module: &Module) {
    build_convert_func(module, FormType::None, &|_: &mut ConvertGenerator| {});
    build_convert_func(module, FormType::Amplitude, &amplitude_converter::amplitude);
    build_convert_func(module, FormType::Beats, &beats_converter::beats);
    build_convert_func(module, FormType::Control, &control_converter::control);
    build_convert_func(module, FormType::Db, &db_converter::db);
    build_convert_func(module, FormType::Frequency, &frequency_converter::frequency);
    build_convert_func(module, FormType::Note, &note_converter::note);
    build_convert_func(
        module,
        FormType::Oscillator,
        &oscillator_converter::oscillator,
    );
    build_convert_func(module, FormType::Q, &q_converter::q);
    build_convert_func(module, FormType::Samples, &samples_converter::samples);
    build_convert_func(module, FormType::Seconds, &seconds_converter::seconds);
}

pub fn build_convert_direct(
    builder: &mut Builder,
    module: &Module,
    source: &NumValue,
    target_form: FormType,
) -> StructValue {
    let convert_func = get_convert_func(module, target_form);
    builder
        .build_call(&convert_func, &[&source.val], "num.converted", false)
        .left()
        .unwrap()
        .into_struct_value()
}

pub fn build_convert(
    alloca_builder: &mut Builder,
    builder: &mut Builder,
    module: &Module,
    source: &NumValue,
    target_form: FormType,
) -> NumValue {
    let context = module.get_context();
    let result_num = NumValue::new_undef(&context, alloca_builder);
    let converted_num = build_convert_direct(builder, module, source, target_form);
    builder.build_store(&result_num.val, &converted_num);
    result_num
}
