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
use codegen::values::NumValue;
use codegen::{build_context_function, util, BuilderContext, TargetProperties};
use inkwell::basic_block::BasicBlock;
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::values::{FunctionValue, IntValue, StructValue, VectorValue};

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
        let new_block = self
            .context
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
            num_type.fn_type(&[&num_type], false),
        )
    })
}

pub fn build_convert_func(
    module: &Module,
    target: &TargetProperties,
    target_form: FormType,
    build_func: &Fn(&mut ConvertGenerator),
) {
    let func = get_convert_func(module, target_form);
    build_context_function(module, func, target, &|ctx: BuilderContext| {
        let input_val = func.get_nth_param(0).unwrap().into_struct_value();
        let result_num = NumValue::new_undef(ctx.context, ctx.allocb);

        let input_ptr = ctx.allocb.build_alloca(&input_val.get_type(), "in.ptr");
        ctx.b.build_store(&input_ptr, &input_val);
        let input_num = NumValue::new(input_ptr);

        let input_form = input_num.get_form(ctx.b);
        let input_vec = input_num.get_vec(ctx.b);

        result_num.set_form(
            ctx.b,
            &ctx.context.i8_type().const_int(target_form as u64, false),
        );

        // build up each switch block
        let end_block = ctx.context.append_basic_block(&ctx.func, "end");
        let default_block = ctx.context.append_basic_block(&ctx.func, "default");

        let mut converter = PrivateGenerator {
            func: &ctx.func,
            input_vec: &input_vec,
            result_num: &result_num,
            end_block: &end_block,
            context: ctx.context,
            module,
            branches: Vec::new(),
        };
        build_func(&mut converter);

        let switch_cases: Vec<_> = converter
            .branches
            .iter()
            .map(|&(ref form_type, ref block)| (form_type, block))
            .collect();
        ctx.b
            .build_switch(&input_form, &default_block, &switch_cases);

        ctx.b.position_at_end(&default_block);
        result_num.set_vec(ctx.b, &input_vec);
        ctx.b.build_unconditional_branch(&end_block);

        ctx.b.position_at_end(&end_block);
        let return_val = result_num.load(ctx.b);
        ctx.b.build_return(Some(&return_val));
    });
}

pub fn build_funcs(module: &Module, target: &TargetProperties) {
    build_convert_func(
        module,
        target,
        FormType::None,
        &|_: &mut ConvertGenerator| {},
    );
    build_convert_func(
        module,
        target,
        FormType::Amplitude,
        &amplitude_converter::amplitude,
    );
    build_convert_func(module, target, FormType::Beats, &beats_converter::beats);
    build_convert_func(
        module,
        target,
        FormType::Control,
        &control_converter::control,
    );
    build_convert_func(module, target, FormType::Db, &db_converter::db);
    build_convert_func(
        module,
        target,
        FormType::Frequency,
        &frequency_converter::frequency,
    );
    build_convert_func(module, target, FormType::Note, &note_converter::note);
    build_convert_func(
        module,
        target,
        FormType::Oscillator,
        &oscillator_converter::oscillator,
    );
    build_convert_func(module, target, FormType::Q, &q_converter::q);
    build_convert_func(
        module,
        target,
        FormType::Samples,
        &samples_converter::samples,
    );
    build_convert_func(
        module,
        target,
        FormType::Seconds,
        &seconds_converter::seconds,
    );
}

pub fn build_convert_direct(
    builder: &mut Builder,
    module: &Module,
    source: &NumValue,
    target_form: FormType,
) -> StructValue {
    let convert_func = get_convert_func(module, target_form);
    let loaded_val = builder.build_load(&source.val, "in").into_struct_value();
    builder
        .build_call(&convert_func, &[&loaded_val], "num.converted", true)
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
