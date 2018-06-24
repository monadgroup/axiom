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
}

pub fn build_context_function(module: &Module, function: FunctionValue, cb: &Fn(BuilderContext)) {
    let context = module.get_context();

    let alloca_block = context.append_basic_block(&function, "alloca");
    let mut alloca_builder = context.create_builder();
    alloca_builder.position_at_end(&alloca_block);

    let main_block = context.append_basic_block(&function, "main");
    let mut builder = context.create_builder();
    builder.position_at_end(&main_block);

    cb(BuilderContext {
        context: &context,
        module,
        func: function,
        allocb: &mut alloca_builder,
        b: &mut builder,
    });

    // ensure the alloca block jumps to the main block
    alloca_builder.build_unconditional_branch(&main_block);
}
