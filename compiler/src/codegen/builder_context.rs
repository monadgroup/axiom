use crate::codegen::TargetProperties;
use inkwell::attribute::AttrKind;
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::FunctionValue;

pub struct BuilderContext<'a> {
    pub context: &'a Context,
    pub module: &'a Module,
    pub func: FunctionValue,
    pub allocb: &'a mut Builder,
    pub b: &'a mut Builder,
    pub target: &'a TargetProperties,
}

fn get_target_feature_string() -> String {
    // we need SSE4.1 at a minimum, so dynamically enable SSE4.2, AVX, and AVX2 if we can
    let mut base_features = "+x87,+mmx,+sse,+sse2,+sse3,+ssse3,+sse4.1".to_string();
    if is_x86_feature_detected!("sse4.2") {
        base_features.push_str(",+sse4.2");
    }
    if is_x86_feature_detected!("avx") {
        base_features.push_str(",+avx");
    }
    if is_x86_feature_detected!("avx2") {
        base_features.push_str(",+avx2");
    }
    base_features
}

lazy_static! {
    static ref TARGET_FEATURE_STR: String = get_target_feature_string();
}

pub fn build_context_function(
    module: &Module,
    function: FunctionValue,
    target: &TargetProperties,
    cb: &Fn(BuilderContext),
) {
    let context = module.get_context();

    // assign some useful attributes to the function
    function
        .add_attribute(context.get_string_attr("correctly-rounded-divide-sqrt-fp-math", "false"));
    function.add_attribute(context.get_string_attr("disable-tail-calls", "false"));
    function.add_attribute(context.get_string_attr("less-precise-fpmad", "false"));
    function.add_attribute(context.get_string_attr("no-frame-pointer-elim", "false"));
    function.add_attribute(context.get_string_attr("no-infs-fp-math", "true"));
    function.add_attribute(context.get_string_attr("no-jump-tables", "false"));
    function.add_attribute(context.get_string_attr("no-nans-fp-math", "true"));
    function.add_attribute(context.get_string_attr("no-signed-zeros-fp-math", "true"));
    function.add_attribute(context.get_string_attr("no-trapping-math", "true"));
    function.add_attribute(context.get_string_attr("denorms-are-zero", "true"));
    function.add_attribute(context.get_string_attr("denormal-fp-math", "positive-zero"));
    function.add_attribute(context.get_string_attr("target-features", &TARGET_FEATURE_STR));
    function.add_attribute(context.get_string_attr("unsafe-fp-math", "true"));
    function.add_attribute(context.get_string_attr("use-soft-float", "false"));

    function.add_attribute(context.get_enum_attr(AttrKind::NoRecurse, 0));
    function.add_attribute(context.get_enum_attr(AttrKind::NoUnwind, 0));

    if target.min_size {
        //function.add_attribute(context.get_enum_attr(AttrKind::MinSize, 0));
        //function.add_attribute(context.get_enum_attr(AttrKind::OptimizeForSize, 0));
    }

    let alloca_block = context.append_basic_block(&function, "alloca");
    let mut alloca_builder = context.create_builder();
    alloca_builder.set_fast_math_all();
    alloca_builder.position_at_end(&alloca_block);

    let main_block = context.append_basic_block(&function, "main");
    let mut builder = context.create_builder();
    builder.set_fast_math_all();
    builder.position_at_end(&main_block);

    cb(BuilderContext {
        context: &context,
        module,
        func: function,
        allocb: &mut alloca_builder,
        b: &mut builder,
        target,
    });

    // ensure the alloca block jumps to the main block
    alloca_builder.build_unconditional_branch(&main_block);
}
