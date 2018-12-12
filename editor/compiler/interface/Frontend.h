#pragma once

#include <cstdint>

namespace MaximFrontend {
    using MaximError = void;
    using MaximErrorRef = MaximError;

    using MaximRuntime = void;
    using MaximRuntimeRef = MaximRuntime;

    using MaximTransaction = void;
    using MaximTransactionRef = MaximTransaction;

    using MaximVarType = void;
    using MaximVarTypeRef = MaximVarType;
    using MaximConstantValue = void;
    using MaximConstantValueRef = MaximConstantValue;

    using MaximRootRef = void;
    using MaximSurfaceRef = void;
    using MaximNodeRef = void;
    using MaximBlock = void;
    using MaximBlockRef = MaximBlock;

    using MaximBlockControlRef = void;

    using MaximValueGroupSource = void;

    struct SourcePos {
        ptrdiff_t line;
        ptrdiff_t column;
    };

    struct SourceRange {
        SourcePos front;
        SourcePos back;
    };

    struct ControlPointers {
        void *value;
        void *data;
        void *shared;
        void *ui;
    };

    extern "C" {
    void maxim_initialize();

    MaximRuntime *maxim_create_runtime(bool includeUi, bool minSize);
    void maxim_destroy_runtime(MaximRuntime *);

    uint64_t maxim_allocate_id(MaximRuntimeRef *runtime);
    void maxim_run_update(MaximRuntimeRef *runtime);
    void maxim_set_bpm(MaximRuntimeRef *runtime, double bpm);
    double maxim_get_bpm(MaximRuntimeRef *runtime);
    void maxim_set_sample_rate(MaximRuntimeRef *runtime, double sample_rate);
    double maxim_get_sample_rate(MaximRuntimeRef *runtime);
    uint64_t *maxim_get_profile_times_ptr(MaximRuntimeRef *runtime);
    bool maxim_is_node_extracted(MaximRuntimeRef *runtime, uint64_t surface, size_t node);
    void maxim_convert_num(MaximRuntimeRef *runtime, void *result, uint8_t targetForm, const void *input);

    void *maxim_get_portal_ptr(MaximRuntimeRef *runtime, size_t portal);
    void *maxim_get_root_ptr(MaximRuntimeRef *runtime);
    void *maxim_get_node_ptr(MaximRuntimeRef *runtime, uint64_t surface, void *surface_ptr, size_t node);
    uint32_t *maxim_get_extracted_bitmask_ptr(MaximRuntimeRef *runtime, uint64_t surface, void *surface_ptr,
                                              size_t node);
    void *maxim_get_surface_ptr(void *node_ptr);
    void *maxim_get_block_ptr(void *block_ptr);
    ControlPointers maxim_get_control_ptrs(MaximRuntimeRef *runtime, uint64_t block, void *block_ptr, size_t control);

    void maxim_destroy_string(const char *);

    MaximTransaction *maxim_create_transaction();
    void maxim_destroy_transaction(MaximTransaction *);
    void maxim_print_transaction_to_stdout(MaximTransactionRef *);

    MaximVarType *maxim_vartype_num();
    MaximVarType *maxim_vartype_midi();
    MaximVarType *maxim_vartype_tuple(MaximVarType **subtypes, size_t subtype_count);
    MaximVarType *maxim_vartype_array(MaximVarType *subtype);
    MaximVarType *maxim_vartype_of_control(uint8_t control_type);
    MaximVarType *maxim_vartype_clone(MaximVarTypeRef *base);
    void maxim_destroy_vartype(MaximVarType *);

    MaximConstantValue *maxim_constant_num(double left, double right, uint8_t form);
    MaximConstantValue *maxim_constant_tuple(MaximConstantValue **items, size_t item_count);
    MaximConstantValue *maxim_constant_clone(MaximConstantValueRef *base);
    void maxim_destroy_constant(MaximConstantValue *);

    MaximRootRef *maxim_build_root(MaximTransactionRef *transaction);
    void maxim_build_root_socket(MaximRootRef *root, MaximVarType *vartype);

    MaximSurfaceRef *maxim_build_surface(MaximTransactionRef *transaction, uint64_t id, const char *name);

    MaximValueGroupSource *maxim_valuegroupsource_none();
    MaximValueGroupSource *maxim_valuegroupsource_socket(size_t index);
    MaximValueGroupSource *maxim_valuegroupsource_default(MaximConstantValue *value);
    MaximValueGroupSource *maxim_valuegroupsource_clone(MaximValueGroupSource *base);
    void maxim_destroy_valuegroupsource(MaximValueGroupSource *);
    void maxim_build_value_group(MaximSurfaceRef *surface, MaximVarType *vartype, MaximValueGroupSource *source);

    MaximNodeRef *maxim_build_custom_node(MaximSurfaceRef *surface, uint64_t block_id);
    MaximNodeRef *maxim_build_group_node(MaximSurfaceRef *surface, uint64_t surface_id);
    void maxim_build_value_socket(MaximNodeRef *node, size_t group_id, bool value_written, bool value_read,
                                  bool is_extractor);

    void maxim_build_block(MaximTransactionRef *transaction, MaximBlock *);
    bool maxim_compile_block(uint64_t id, const char *name, const char *code, MaximBlock **success_block_out,
                             MaximError **fail_error_out);
    void maxim_destroy_block(MaximBlock *);
    MaximBlock *maxim_block_clone(MaximBlockRef *);

    const char *maxim_error_get_description(MaximErrorRef *);
    SourceRange maxim_error_get_range(MaximErrorRef *);
    void maxim_destroy_error(MaximError *);

    size_t maxim_block_get_control_count(MaximBlockRef *block);
    MaximBlockControlRef *maxim_block_get_control(MaximBlockRef *block, size_t index);
    const char *maxim_control_get_name(MaximBlockControlRef *control);
    uint8_t maxim_control_get_type(MaximBlockControlRef *control);
    bool maxim_control_get_written(MaximBlockControlRef *control);
    bool maxim_control_get_read(MaximBlockControlRef *control);

    void maxim_commit(MaximRuntimeRef *runtime, MaximTransaction *transaction);

    size_t maxim_get_function_table_size();
    const char *maxim_get_function_table_entry(size_t index);
    }
}
