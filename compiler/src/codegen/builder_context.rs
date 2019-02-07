use crate::codegen::{OptimizationLevel, TargetProperties};
use crate::util::feature_level::{get_target_feature_string, FEATURE_LEVEL};
use inkwell::attribute::AttrKind;
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::values::FunctionValue;
use lazy_static::lazy_static;

pub struct BuilderContext<'a> {
    pub context: &'a Context,
    pub module: &'a Module,
    pub func: FunctionValue,
    pub allocb: &'a mut Builder,
    pub b: &'a mut Builder,
    pub target: &'a TargetProperties,
}

lazy_static! {
    pub static ref TARGET_FEATURE_STR: String = get_target_feature_string(*FEATURE_LEVEL);
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
    function.add_attribute(context.get_string_attr(
        "target-features",
        target.machine.get_feature_string().to_str().unwrap(),
    ));
    function.add_attribute(context.get_string_attr("unsafe-fp-math", "true"));
    function.add_attribute(context.get_string_attr("use-soft-float", "false"));

    function.add_attribute(context.get_enum_attr(AttrKind::NoRecurse, 0));
    function.add_attribute(context.get_enum_attr(AttrKind::NoUnwind, 0));

    if target.optimization_level == OptimizationLevel::MinSize {
        function.add_attribute(context.get_enum_attr(AttrKind::OptimizeForSize, 0));
    }
    if target.optimization_level == OptimizationLevel::AggressiveSize {
        function.add_attribute(context.get_enum_attr(AttrKind::MinSize, 0));
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
