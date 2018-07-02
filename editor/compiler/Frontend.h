#pragma once
#include <cstdint>

namespace MaximFrontend {
    using MaximError = void;

    using MaximRuntime = void;
    using MaximRuntimeRef = MaximRuntime;

    using MaximTransaction = void;
    using MaximTransactionRef = MaximTransaction;

    using MaximVarType = void;
    using MaximVarTypeRef = MaximVarType;
    using MaximConstantValue = void;
    using MaximConstantValueRef = MaximConstantValue;

    using MaximSurfaceRef = void;
    using MaximNodeRef = void;
    using MaximBlock = void;
    using MaximBlockRef = MaximBlock;

    using MaximBlockControlRef = void;

    using MaximValueGroupSource = void;

    extern "C" {
        void maxim_initialize();

        MaximRuntime *maxim_create_runtime();
        void maxim_destroy_runtime(MaximRuntime *);

        uint64_t maxim_allocate_id(MaximRuntimeRef *runtime);
        void maxim_destroy_string(const char *);

        MaximTransaction *maxim_create_transaction();

        MaximVarType *maxim_vartype_num();
        MaximVarType *maxim_vartype_midi();
        MaximVarType *maxim_vartype_tuple(MaximVarType **subtypes, size_t subtype_count);
        MaximVarType *maxim_vartype_array(MaximVarType *subtype);
        MaximVarType *maxim_vartype_clone(MaximVarTypeRef *base);

        MaximConstantValue *maxim_constant_num(float left, float right, uint8_t form);
        MaximConstantValue *maxim_constant_tuple(MaximConstantValue **items, size_t item_count);
        MaximConstantValue *maxim_constant_clone(MaximConstantValueRef *base);

        MaximSurfaceRef *maxim_build_surface(MaximTransactionRef *transaction, uint64_t id, const char *name);

        MaximValueGroupSource *maxim_valuegroupsource_none();
        MaximValueGroupSource *maxim_valuegroupsource_socket(size_t index);
        MaximValueGroupSource *maxim_valuegroupsource_default(MaximConstantValue *value);
        void maxim_build_value_group(MaximSurfaceRef *surface, MaximVarType *vartype, MaximValueGroupSource *source);

        MaximNodeRef *maxim_build_custom_node(MaximSurfaceRef *surface, uint64_t block_id);
        MaximNodeRef *maxim_build_group_node(MaximSurfaceRef *surface, uint64_t surface_id);
        void maxim_build_value_socket(MaximNodeRef *node, size_t group_id, bool value_written, bool value_read, bool is_extractor);

        bool maxim_compile_block(uint64_t id, const char *name, const char *code, MaximBlock *success_block_out, MaximError *fail_error_out);
        void maxim_destroy_error(MaximError *);

        size_t maxim_block_get_control_count(MaximBlockRef *block);
        MaximBlockControlRef *maxim_block_get_control(MaximBlockRef *block, size_t index);
        const char *maxim_control_get_name(MaximBlockControlRef *control);
        uint8_t maxim_control_get_type(MaximBlockControlRef *control);
        bool maxim_control_get_written(MaximBlockControlRef *control);
        bool maxim_control_get_read(MaximBlockControlRef *control);

        void maxim_commit(MaximRuntimeRef *runtime, MaximTransaction *transaction);
    }
}
