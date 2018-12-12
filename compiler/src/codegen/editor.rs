use crate::ast::FormType;
use crate::codegen::{
    build_context_function, converters, util, values, BuilderContext, TargetProperties,
};
use inkwell::attribute::AttrKind;
use inkwell::module::{Linkage, Module};
use inkwell::AddressSpace;

const FORM_TYPES: [FormType; 9] = [
    FormType::Amplitude,
    FormType::Beats,
    FormType::Control,
    FormType::Db,
    FormType::Frequency,
    FormType::Note,
    FormType::Q,
    FormType::Samples,
    FormType::Seconds,
];

pub fn build_convert_num_func(module: &Module, target: &TargetProperties, name: &str) {
    let func = util::get_or_create_func(module, name, false, &|| {
        let context = &module.get_context();
        let num_type = values::NumValue::get_type(context);
        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(
                &[
                    &num_type.ptr_type(AddressSpace::Generic), // return value
                    &context.i8_type(),                        // target form
                    &num_type.ptr_type(AddressSpace::Generic), // input number
                ],
                false,
            ),
        )
    });
    func.add_param_attribute(
        0,
        module.get_context().get_enum_attr(AttrKind::StructRet, 1),
    );

    build_context_function(module, func, target, &|ctx: BuilderContext| {
        let output_num =
            values::NumValue::new(ctx.func.get_nth_param(0).unwrap().into_pointer_value());
        let target_form = ctx.func.get_nth_param(1).unwrap().into_int_value();
        let input_num =
            values::NumValue::new(ctx.func.get_nth_param(2).unwrap().into_pointer_value());

        let mut case_builder = ctx.context.create_builder();
        case_builder.set_fast_math_all();

        let default_block = ctx.context.append_basic_block(&ctx.func, "default");
        case_builder.position_at_end(&default_block);
        input_num.copy_to(&mut case_builder, ctx.module, &output_num);
        case_builder.build_return(None);

        // build a switch based on the target form
        let mut switch_cases = Vec::new();
        for &form_type in FORM_TYPES.iter() {
            let form_block = ctx
                .context
                .append_basic_block(&ctx.func, &format!("form.{}", form_type));
            case_builder.position_at_end(&form_block);

            let result_num = converters::build_convert_direct(
                &mut case_builder,
                ctx.module,
                &input_num,
                form_type,
            );
            case_builder.build_store(&output_num.val, &result_num);
            case_builder.build_return(None);

            let form_num = ctx.context.i8_type().const_int(form_type as u64, false);
            switch_cases.push((form_num, form_block));
        }
        let switch_refs: Vec<_> = switch_cases.iter().map(|&(ref a, ref b)| (a, b)).collect();
        ctx.b
            .build_switch(&target_form, &default_block, &switch_refs);
    });
}
