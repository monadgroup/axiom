mod audio_control;
mod audio_extract_control;
mod control_context;
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

use self::control_context::{ControlContext, ControlUiContext};
use crate::ast::{ControlField, ControlType};
use crate::codegen::{
    build_context_function, util, values, BuilderContext, LifecycleFunc, TargetProperties,
};
use crate::mir::{ControlInitializer, VarType};
use inkwell::attribute::AttrKind;
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::{Linkage, Module};
use inkwell::types::StructType;
use inkwell::values::{FunctionValue, PointerValue, StructValue};
use inkwell::AddressSpace;

pub type ControlFieldGeneratorCb = Fn(&mut ControlContext, PointerValue);

pub trait ControlFieldGenerator {
    fn generate(
        &self,
        field: ControlField,
        get_cb: &ControlFieldGeneratorCb,
        set_cb: &ControlFieldGeneratorCb,
    );
}

#[derive(Clone, Copy)]
pub struct ControlPointers {
    pub group: PointerValue,
    pub const_dat: PointerValue,
    pub data: PointerValue,
    pub shared: PointerValue,
}

struct PrivateGenerator<'a> {
    module: &'a Module,
    target: &'a TargetProperties,
}

impl<'a> ControlFieldGenerator for PrivateGenerator<'a> {
    fn generate(
        &self,
        field: ControlField,
        get_cb: &ControlFieldGeneratorCb,
        set_cb: &ControlFieldGeneratorCb,
    ) {
        let (getter_func, getter_pass_by_val) = get_field_getter_func(self.module, field);
        let (setter_func, setter_pass_by_val) = get_field_setter_func(self.module, field);

        build_field_getter_func(
            self.module,
            getter_func,
            getter_pass_by_val,
            self.target,
            get_cb,
        );
        build_field_setter_func(
            self.module,
            setter_func,
            setter_pass_by_val,
            self.target,
            set_cb,
        );
    }
}

pub fn get_group_type(context: &Context, control_type: ControlType) -> StructType {
    values::remap_type(context, &VarType::of_control_value(control_type))
}

pub fn get_field_type(context: &Context, field: ControlField) -> StructType {
    values::remap_type(context, &VarType::of_control_field(field))
}

macro_rules! map_controls {
    ($($enum_name:ident => $class_name:ident),*) => (
        pub fn get_constant_type(context: &Context, control_type: ControlType) -> StructType {
            match control_type {
                $( ControlType::$enum_name => $class_name::constant_type(context), )*
            }
        }

        pub fn get_constant_value(context: &Context, control_type: ControlType, initializer: &ControlInitializer) -> StructValue {
            match control_type {
                $( ControlType::$enum_name => $class_name::constant_value(context, initializer), )*
            }
        }

        pub fn get_data_type(context: &Context, control_type: ControlType) -> StructType {
            match control_type {
                $( ControlType::$enum_name => $class_name::data_type(context), )*
            }
        }

        pub fn get_shared_data_type(context: &Context, control_type: ControlType) -> StructType {
            match control_type {
                $( ControlType::$enum_name => $class_name::shared_data_type(context), )*
            }
        }

        pub fn get_ui_type(context: &Context, control_type: ControlType) -> StructType {
            match control_type {
                $( ControlType::$enum_name => $class_name::ui_type(context), )*
            }
        }

        pub fn build_funcs(module: &Module, target: &TargetProperties) {
            $( $class_name::build_funcs(module, target); )*
        }
    )
}

map_controls! {
    Audio => AudioControl,
    Midi => MidiControl,
    AudioExtract => AudioExtractControl,
    MidiExtract => MidiExtractControl,
    Graph => GraphControl,
    Roll => RollControl,
    Scope => ScopeControl
}

fn get_lifecycle_func(
    module: &Module,
    control_type: ControlType,
    lifecycle: LifecycleFunc,
) -> FunctionValue {
    let func_name = format!("maxim.control.{}.{}", control_type, lifecycle);
    let func = util::get_or_create_func(module, &func_name, true, &|| {
        let context = module.get_context();
        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(
                &[
                    &get_group_type(&context, control_type).ptr_type(AddressSpace::Generic),
                    &get_constant_type(&context, control_type).ptr_type(AddressSpace::Generic),
                    &get_data_type(&context, control_type).ptr_type(AddressSpace::Generic),
                    &get_shared_data_type(&context, control_type).ptr_type(AddressSpace::Generic),
                ],
                false,
            ),
        )
    });
    let context = module.get_context();
    func.add_param_attribute(0, context.get_enum_attr(AttrKind::NoAlias, 1));
    func.add_param_attribute(1, context.get_enum_attr(AttrKind::NoAlias, 1));
    func.add_param_attribute(2, context.get_enum_attr(AttrKind::NoAlias, 1));
    func.add_param_attribute(3, context.get_enum_attr(AttrKind::NoAlias, 1));
    func
}

fn get_ui_lifecycle_func(
    module: &Module,
    control_type: ControlType,
    lifecycle: LifecycleFunc,
) -> FunctionValue {
    let func_name = format!("maxim.control.{}.ui_{}", control_type, lifecycle);
    let func = util::get_or_create_func(module, &func_name, true, &|| {
        let context = module.get_context();
        (
            Linkage::ExternalLinkage,
            context.void_type().fn_type(
                &[
                    &get_group_type(&context, control_type).ptr_type(AddressSpace::Generic),
                    &get_constant_type(&context, control_type).ptr_type(AddressSpace::Generic),
                    &get_data_type(&context, control_type).ptr_type(AddressSpace::Generic),
                    &get_shared_data_type(&context, control_type).ptr_type(AddressSpace::Generic),
                    &get_ui_type(&context, control_type).ptr_type(AddressSpace::Generic),
                ],
                false,
            ),
        )
    });
    let context = module.get_context();
    func.add_param_attribute(0, context.get_enum_attr(AttrKind::NoAlias, 1));
    func.add_param_attribute(1, context.get_enum_attr(AttrKind::NoAlias, 1));
    func.add_param_attribute(2, context.get_enum_attr(AttrKind::NoAlias, 1));
    func.add_param_attribute(3, context.get_enum_attr(AttrKind::NoAlias, 1));
    func.add_param_attribute(4, context.get_enum_attr(AttrKind::NoAlias, 1));
    func
}

fn get_field_getter_func(module: &Module, field: ControlField) -> (FunctionValue, bool) {
    let var_type = VarType::of_control_field(field);
    let pass_by_val = values::pass_type_by_val(&var_type);

    let func = util::get_or_create_func(
        module,
        &format!("maxim.control.{}.getter", field),
        true,
        &|| {
            let control_type = ControlType::from(field);

            let context = module.get_context();
            let value_type = values::remap_type(&context, &var_type);
            let group_type = get_group_type(&context, control_type).ptr_type(AddressSpace::Generic);
            let const_type =
                get_constant_type(&context, control_type).ptr_type(AddressSpace::Generic);
            let data_type = get_data_type(&context, control_type).ptr_type(AddressSpace::Generic);
            let shared_data_type =
                get_shared_data_type(&context, control_type).ptr_type(AddressSpace::Generic);

            let func_type = if pass_by_val {
                value_type.fn_type(
                    &[&group_type, &const_type, &data_type, &shared_data_type],
                    false,
                )
            } else {
                context.void_type().fn_type(
                    &[
                        &value_type.ptr_type(AddressSpace::Generic),
                        &group_type,
                        &const_type,
                        &data_type,
                        &shared_data_type,
                    ],
                    false,
                )
            };

            (Linkage::ExternalLinkage, func_type)
        },
    );

    let context = module.get_context();

    if pass_by_val {
        func.add_param_attribute(0, context.get_enum_attr(AttrKind::NoAlias, 1));
        func.add_param_attribute(1, context.get_enum_attr(AttrKind::NoAlias, 1));
        func.add_param_attribute(2, context.get_enum_attr(AttrKind::NoAlias, 1));
        func.add_param_attribute(3, context.get_enum_attr(AttrKind::NoAlias, 1));
    } else {
        func.add_param_attribute(0, context.get_enum_attr(AttrKind::StructRet, 1));
        func.add_param_attribute(1, context.get_enum_attr(AttrKind::NoAlias, 1));
        func.add_param_attribute(2, context.get_enum_attr(AttrKind::NoAlias, 1));
        func.add_param_attribute(3, context.get_enum_attr(AttrKind::NoAlias, 1));
        func.add_param_attribute(4, context.get_enum_attr(AttrKind::NoAlias, 1));
    }
    (func, pass_by_val)
}

fn get_field_setter_func(module: &Module, field: ControlField) -> (FunctionValue, bool) {
    let var_type = VarType::of_control_field(field);
    let pass_by_val = values::pass_type_by_val(&var_type);

    let func = util::get_or_create_func(
        module,
        &format!("maxim.control.{}.setter", field),
        true,
        &|| {
            let control_type = ControlType::from(field);

            let context = module.get_context();
            let value_type = values::remap_type(&context, &var_type);
            let group_type = get_group_type(&context, control_type).ptr_type(AddressSpace::Generic);
            let const_type =
                get_constant_type(&context, control_type).ptr_type(AddressSpace::Generic);
            let data_type = get_data_type(&context, control_type).ptr_type(AddressSpace::Generic);
            let shared_data_type =
                get_shared_data_type(&context, control_type).ptr_type(AddressSpace::Generic);

            let func_type = if pass_by_val {
                context.void_type().fn_type(
                    &[
                        &group_type,
                        &const_type,
                        &data_type,
                        &shared_data_type,
                        &value_type,
                    ],
                    false,
                )
            } else {
                context.void_type().fn_type(
                    &[
                        &group_type,
                        &const_type,
                        &data_type,
                        &shared_data_type,
                        &value_type.ptr_type(AddressSpace::Generic),
                    ],
                    false,
                )
            };

            (Linkage::ExternalLinkage, func_type)
        },
    );

    let context = module.get_context();
    func.add_param_attribute(0, context.get_enum_attr(AttrKind::NoAlias, 1));
    func.add_param_attribute(1, context.get_enum_attr(AttrKind::NoAlias, 1));
    func.add_param_attribute(2, context.get_enum_attr(AttrKind::NoAlias, 1));
    func.add_param_attribute(3, context.get_enum_attr(AttrKind::NoAlias, 1));
    if !pass_by_val {
        func.add_param_attribute(4, context.get_enum_attr(AttrKind::NoAlias, 1));
    }

    (func, pass_by_val)
}

pub fn build_field_get(
    module: &Module,
    builder: &mut Builder,
    field: ControlField,
    ptrs: ControlPointers,
    out_val: PointerValue,
) {
    let (func, pass_by_val) = get_field_getter_func(module, field);

    if pass_by_val {
        let get_val = builder
            .build_call(
                &func,
                &[&ptrs.group, &ptrs.const_dat, &ptrs.data, &ptrs.shared],
                "field.get",
                true,
            )
            .left()
            .unwrap();
        builder.build_store(&out_val, &get_val);
    } else {
        builder.build_call(
            &func,
            &[
                &out_val,
                &ptrs.group,
                &ptrs.const_dat,
                &ptrs.data,
                &ptrs.shared,
            ],
            "",
            true,
        );
    }
}

pub fn build_field_set(
    module: &Module,
    builder: &mut Builder,
    field: ControlField,
    ptrs: ControlPointers,
    in_val: PointerValue,
) {
    let (func, pass_by_val) = get_field_setter_func(module, field);

    let in_norm_val = if pass_by_val {
        builder.build_load(&in_val, "in")
    } else {
        in_val.into()
    };

    builder.build_call(
        &func,
        &[
            &ptrs.group,
            &ptrs.const_dat,
            &ptrs.data,
            &ptrs.shared,
            &in_norm_val,
        ],
        "",
        true,
    );
}

pub fn build_lifecycle_call(
    module: &Module,
    builder: &mut Builder,
    control_type: ControlType,
    lifecycle: LifecycleFunc,
    ptrs: ControlPointers,
) {
    let func = get_lifecycle_func(module, control_type, lifecycle);
    builder.build_call(
        &func,
        &[&ptrs.group, &ptrs.const_dat, &ptrs.data, &ptrs.shared],
        "",
        true,
    );
}

pub fn build_ui_lifecycle_call(
    module: &Module,
    builder: &mut Builder,
    control_type: ControlType,
    lifecycle: LifecycleFunc,
    ptrs: ControlPointers,
    ui_ptr: PointerValue,
) {
    let func = get_ui_lifecycle_func(module, control_type, lifecycle);
    builder.build_call(
        &func,
        &[
            &ptrs.group,
            &ptrs.const_dat,
            &ptrs.data,
            &ptrs.shared,
            &ui_ptr,
        ],
        "",
        true,
    );
}

fn build_lifecycle_func(
    module: &Module,
    control: ControlType,
    target: &TargetProperties,
    lifecycle: LifecycleFunc,
    builder: &Fn(&mut ControlContext),
) {
    let func = get_lifecycle_func(module, control, lifecycle);
    build_context_function(module, func, target, &|ctx: BuilderContext| {
        let val_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();
        let const_ptr = ctx.func.get_nth_param(1).unwrap().into_pointer_value();
        let data_ptr = ctx.func.get_nth_param(2).unwrap().into_pointer_value();
        let shared_ptr = ctx.func.get_nth_param(3).unwrap().into_pointer_value();
        let mut control_context = ControlContext {
            ctx,
            val_ptr,
            const_ptr,
            data_ptr,
            shared_ptr,
        };
        builder(&mut control_context);

        control_context.ctx.b.build_return(None);
    });
}

fn build_ui_lifecycle_func(
    module: &Module,
    control: ControlType,
    target: &TargetProperties,
    lifecycle: LifecycleFunc,
    builder: &Fn(&mut ControlUiContext),
) {
    let func = get_ui_lifecycle_func(module, control, lifecycle);
    build_context_function(module, func, target, &|ctx: BuilderContext| {
        let val_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();
        let const_ptr = ctx.func.get_nth_param(1).unwrap().into_pointer_value();
        let data_ptr = ctx.func.get_nth_param(2).unwrap().into_pointer_value();
        let shared_ptr = ctx.func.get_nth_param(3).unwrap().into_pointer_value();
        let ui_ptr = ctx.func.get_nth_param(4).unwrap().into_pointer_value();
        let mut control_context = ControlUiContext {
            ctx,
            val_ptr,
            const_ptr,
            data_ptr,
            shared_ptr,
            ui_ptr,
        };
        builder(&mut control_context);

        control_context.ctx.b.build_return(None);
    });
}

fn build_field_getter_func(
    module: &Module,
    func: FunctionValue,
    pass_by_val: bool,
    target: &TargetProperties,
    builder: &ControlFieldGeneratorCb,
) {
    build_context_function(module, func, target, &|ctx: BuilderContext| {
        let mut param_iter = ctx.func.params();

        let field_ptr = if pass_by_val {
            ctx.allocb
                .build_alloca(&ctx.func.get_return_type(), "ret.ptr")
        } else {
            param_iter.next().unwrap().into_pointer_value()
        };
        let val_ptr = param_iter.next().unwrap().into_pointer_value();
        let const_ptr = param_iter.next().unwrap().into_pointer_value();
        let data_ptr = param_iter.next().unwrap().into_pointer_value();
        let shared_ptr = param_iter.next().unwrap().into_pointer_value();
        let mut control_context = ControlContext {
            ctx,
            val_ptr,
            const_ptr,
            data_ptr,
            shared_ptr,
        };
        builder(&mut control_context, field_ptr);

        if pass_by_val {
            let loaded_val = control_context.ctx.b.build_load(&field_ptr, "ret");
            control_context.ctx.b.build_return(Some(&loaded_val));
        } else {
            control_context.ctx.b.build_return(None);
        }
    })
}

fn build_field_setter_func(
    module: &Module,
    func: FunctionValue,
    pass_by_val: bool,
    target: &TargetProperties,
    builder: &ControlFieldGeneratorCb,
) {
    build_context_function(module, func, target, &|ctx: BuilderContext| {
        let val_ptr = ctx.func.get_nth_param(0).unwrap().into_pointer_value();
        let const_ptr = ctx.func.get_nth_param(1).unwrap().into_pointer_value();
        let data_ptr = ctx.func.get_nth_param(2).unwrap().into_pointer_value();
        let shared_ptr = ctx.func.get_nth_param(3).unwrap().into_pointer_value();

        let field_val = ctx.func.get_nth_param(4).unwrap();
        let field_ptr = if pass_by_val {
            let ptr = ctx.allocb.build_alloca(&field_val.get_type(), "val.ptr");
            ctx.b.build_store(&ptr, &field_val);
            ptr
        } else {
            field_val.into_pointer_value()
        };

        let mut control_context = ControlContext {
            ctx,
            val_ptr,
            const_ptr,
            data_ptr,
            shared_ptr,
        };
        builder(&mut control_context, field_ptr);

        control_context.ctx.b.build_return(None);
    })
}

pub trait Control {
    fn control_type() -> ControlType;

    fn constant_type(context: &Context) -> StructType {
        context.struct_type(&[], false)
    }

    fn constant_value(context: &Context, _initializer: &ControlInitializer) -> StructValue {
        context.const_struct(&[], false)
    }

    fn data_type(context: &Context) -> StructType {
        context.struct_type(&[], false)
    }

    fn shared_data_type(context: &Context) -> StructType {
        context.struct_type(&[], false)
    }

    fn ui_type(context: &Context) -> StructType {
        context.struct_type(&[], false)
    }

    fn gen_construct(_control: &mut ControlContext) {}

    fn gen_update(_control: &mut ControlContext) {}

    fn gen_destruct(_control: &mut ControlContext) {}

    fn gen_ui_construct(_control: &mut ControlUiContext) {}

    fn gen_ui_update(_control: &mut ControlUiContext) {}

    fn gen_ui_destruct(_control: &mut ControlUiContext) {}

    fn gen_fields(generator: &ControlFieldGenerator);

    fn build_lifecycle_funcs(module: &Module, target: &TargetProperties) {
        build_lifecycle_func(
            module,
            Self::control_type(),
            target,
            LifecycleFunc::Construct,
            &Self::gen_construct,
        );
        build_lifecycle_func(
            module,
            Self::control_type(),
            target,
            LifecycleFunc::Update,
            &Self::gen_update,
        );
        build_lifecycle_func(
            module,
            Self::control_type(),
            target,
            LifecycleFunc::Destruct,
            &Self::gen_destruct,
        );
    }

    fn build_ui_lifecycle_funcs(module: &Module, target: &TargetProperties) {
        build_ui_lifecycle_func(
            module,
            Self::control_type(),
            target,
            LifecycleFunc::Construct,
            &Self::gen_ui_construct,
        );
        build_ui_lifecycle_func(
            module,
            Self::control_type(),
            target,
            LifecycleFunc::Update,
            &Self::gen_ui_update,
        );
        build_ui_lifecycle_func(
            module,
            Self::control_type(),
            target,
            LifecycleFunc::Destruct,
            &Self::gen_ui_destruct,
        );
    }

    fn build_field_funcs(module: &Module, target: &TargetProperties) {
        Self::gen_fields(&PrivateGenerator { module, target });
    }

    fn build_funcs(module: &Module, target: &TargetProperties) {
        Self::build_lifecycle_funcs(module, target);
        Self::build_field_funcs(module, target);

        if target.include_ui {
            Self::build_ui_lifecycle_funcs(module, target);
        }
    }
}

pub fn default_copy_getter(control: &mut ControlContext, out_val: PointerValue) {
    util::copy_ptr(control.ctx.b, control.ctx.module, control.val_ptr, out_val);
}

pub fn default_copy_setter(control: &mut ControlContext, in_val: PointerValue) {
    util::copy_ptr(control.ctx.b, control.ctx.module, in_val, control.val_ptr);
}
