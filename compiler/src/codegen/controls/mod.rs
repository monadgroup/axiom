mod audio_control;
mod audio_extract_control;
mod graph_control;
mod midi_control;
mod midi_extract_control;
mod roll_control;
mod scope_control;

pub use self::audio_control::AudioControl;
pub use self::audio_extract_control::AudioExtractControl;
pub use self::graph_control::GraphControl;
pub use self::midi_control::MidiControl;
pub use self::midi_extract_control::MidiExtractControl;
pub use self::roll_control::RollControl;
pub use self::scope_control::ScopeControl;

use ast::{ControlField, ControlType};
use codegen::{
    build_context_function, data_analyzer, util, values, BuilderContext, ControlContext,
    ControlUiContext,
};
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::StructType;
use inkwell::values::{FunctionValue, PointerValue};
use inkwell::AddressSpace;
use mir::VarType;
use std::fmt;

pub type ControlFieldGeneratorCb = Fn(&mut ControlContext, PointerValue);

pub trait ControlFieldGenerator {
    fn generate(
        &self,
        field: ControlField,
        get_cb: &ControlFieldGeneratorCb,
        set_cb: &ControlFieldGeneratorCb,
    );
}

struct PrivateGenerator<'a> {
    module: &'a Module,
}

impl<'a> ControlFieldGenerator for PrivateGenerator<'a> {
    fn generate(
        &self,
        field: ControlField,
        get_cb: &ControlFieldGeneratorCb,
        set_cb: &ControlFieldGeneratorCb,
    ) {
        let get_func = get_field_getter_setter_func(
            self.module,
            field,
            &format!("maxim.control.{}.getter", field),
        );
        build_field_func(self.module, get_func, get_cb);

        let set_func = get_field_getter_setter_func(
            self.module,
            field,
            &format!("maxim.control.{}.setter", field),
        );
        build_field_func(self.module, set_func, set_cb);
    }
}

pub fn get_group_type(context: &Context, control_type: ControlType) -> StructType {
    values::remap_type(context, &VarType::of_control_value(&control_type))
}

pub fn get_field_type(context: &Context, field: ControlField) -> StructType {
    values::remap_type(context, &VarType::of_control_field(&field))
}

pub fn get_data_type(context: &Context, control_type: ControlType) -> StructType {
    match control_type {
        ControlType::Audio => AudioControl::data_type(context),
        ControlType::Midi => MidiControl::data_type(context),
        ControlType::AudioExtract => AudioExtractControl::data_type(context),
        ControlType::MidiExtract => MidiExtractControl::data_type(context),
        ControlType::Graph => GraphControl::data_type(context),
        ControlType::Roll => RollControl::data_type(context),
        ControlType::Scope => ScopeControl::data_type(context),
    }
}

pub fn get_ui_type(context: &Context, control_type: ControlType) -> StructType {
    match control_type {
        ControlType::Audio => AudioControl::ui_type(context),
        ControlType::Midi => MidiControl::ui_type(context),
        ControlType::AudioExtract => AudioExtractControl::ui_type(context),
        ControlType::MidiExtract => MidiExtractControl::ui_type(context),
        ControlType::Graph => GraphControl::data_type(context),
        ControlType::Roll => RollControl::data_type(context),
        ControlType::Scope => ScopeControl::data_type(context),
    }
}

enum LifecycleFunc {
    Construct,
    Update,
    Destruct,
}

impl fmt::Display for LifecycleFunc {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            LifecycleFunc::Construct => write!(f, "construct"),
            LifecycleFunc::Update => write!(f, "update"),
            LifecycleFunc::Destruct => write!(f, "destruct"),
        }
    }
}

fn get_lifecycle_func(
    module: &Module,
    control_type: ControlType,
    lifecycle: LifecycleFunc,
) -> FunctionValue {
    let func_name = format!("maxim.control.{}.{}", control_type, lifecycle);
    util::get_or_create_func(module, &func_name, &|| {
        let context = module.get_context();
        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(
                &[
                    &get_group_type(&context, control_type).ptr_type(AddressSpace::Local),
                    &get_data_type(&context, control_type).ptr_type(AddressSpace::Local),
                ],
                false,
            ),
        )
    })
}

fn get_ui_lifecycle_func(
    module: &Module,
    control_type: ControlType,
    lifecycle: LifecycleFunc,
) -> FunctionValue {
    let func_name = format!("maxim.control.{}.ui_{}", control_type, lifecycle);
    util::get_or_create_func(module, &func_name, &|| {
        let context = module.get_context();
        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(
                &[
                    &get_group_type(&context, control_type).ptr_type(AddressSpace::Local),
                    &get_data_type(&context, control_type).ptr_type(AddressSpace::Local),
                    &get_ui_type(&context, control_type).ptr_type(AddressSpace::Local),
                ],
                false,
            ),
        )
    })
}

fn get_field_getter_setter_func(
    module: &Module,
    field: ControlField,
    func_name: &str,
) -> FunctionValue {
    util::get_or_create_func(module, &func_name, &|| {
        let control_type = ControlType::from(field);
        let context = module.get_context();
        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(
                &[
                    &get_group_type(&context, control_type).ptr_type(AddressSpace::Local),
                    &get_data_type(&context, control_type).ptr_type(AddressSpace::Local),
                    &get_field_type(&context, field).ptr_type(AddressSpace::Local),
                ],
                false,
            ),
        )
    })
}

fn build_lifecycle_func(
    module: &Module,
    control: ControlType,
    lifecycle: LifecycleFunc,
    builder: &Fn(&mut ControlContext),
) {
    let func = get_lifecycle_func(module, control, lifecycle);
    build_context_function(module, func, &|ctx: BuilderContext| {
        let val_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();
        let data_ptr = ctx.func.get_nth_param(1).unwrap().into_pointer_value();
        let mut control_context = ControlContext {
            ctx,
            val_ptr,
            data_ptr,
        };
        builder(&mut control_context);

        control_context.ctx.b.build_return(None);
    });
}

fn build_ui_lifecycle_func(
    module: &Module,
    control: ControlType,
    lifecycle: LifecycleFunc,
    builder: &Fn(&mut ControlUiContext),
) {
    let func = get_ui_lifecycle_func(module, control, lifecycle);
    build_context_function(module, func, &|ctx: BuilderContext| {
        let val_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();
        let data_ptr = ctx.func.get_nth_param(1).unwrap().into_pointer_value();
        let ui_ptr = ctx.func.get_nth_param(2).unwrap().into_pointer_value();
        let mut control_context = ControlUiContext {
            ctx,
            val_ptr,
            data_ptr,
            ui_ptr,
        };
        builder(&mut control_context);

        control_context.ctx.b.build_return(None);
    });
}

fn build_field_func(module: &Module, func: FunctionValue, builder: &ControlFieldGeneratorCb) {
    build_context_function(module, func, &|ctx: BuilderContext| {
        let val_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();
        let data_ptr = ctx.func.get_nth_param(1).unwrap().into_pointer_value();
        let field_ptr = ctx.func.get_nth_param(2).unwrap().into_pointer_value();
        let mut control_context = ControlContext {
            ctx,
            val_ptr,
            data_ptr,
        };
        builder(&mut control_context, field_ptr);

        control_context.ctx.b.build_return(None);
    });
}

pub fn build_lifecycle_funcs<T: Control>(module: &Module) {
    build_lifecycle_func(
        module,
        T::control_type(),
        LifecycleFunc::Construct,
        &T::gen_construct,
    );
    build_lifecycle_func(
        module,
        T::control_type(),
        LifecycleFunc::Update,
        &T::gen_update,
    );
    build_lifecycle_func(
        module,
        T::control_type(),
        LifecycleFunc::Destruct,
        &T::gen_destruct,
    );
}

pub fn build_ui_lifecycle_funcs<T: Control>(module: &Module) {
    build_ui_lifecycle_func(
        module,
        T::control_type(),
        LifecycleFunc::Construct,
        &T::gen_ui_construct,
    );
    build_ui_lifecycle_func(
        module,
        T::control_type(),
        LifecycleFunc::Update,
        &T::gen_ui_update,
    );
    build_ui_lifecycle_func(
        module,
        T::control_type(),
        LifecycleFunc::Destruct,
        &T::gen_ui_destruct,
    );
}

pub fn build_field_funcs<T: Control>(module: &Module) {
    T::gen_fields(&PrivateGenerator { module });
}

pub trait Control {
    fn control_type() -> ControlType;

    fn data_type(context: &Context) -> StructType {
        context.struct_type(&[], false)
    }

    fn ui_type(context: &Context) -> StructType {
        context.struct_type(&[], false)
    }

    fn gen_construct(control: &mut ControlContext) {}

    fn gen_update(control: &mut ControlContext) {}

    fn gen_destruct(control: &mut ControlContext) {}

    fn gen_ui_construct(control: &mut ControlUiContext) {}

    fn gen_ui_update(control: &mut ControlUiContext) {}

    fn gen_ui_destruct(control: &mut ControlUiContext) {}

    fn gen_fields(generator: &ControlFieldGenerator);
}

pub fn default_copy_getter(control: &mut ControlContext, out_val: PointerValue) {
    util::copy_ptr(control.ctx.b, control.ctx.module, control.val_ptr, out_val);
}

pub fn default_copy_setter(control: &mut ControlContext, in_val: PointerValue) {
    util::copy_ptr(control.ctx.b, control.ctx.module, in_val, control.val_ptr);
}

pub fn default_null_setter(control: &mut ControlContext, in_val: PointerValue) {}
