#ifndef{{C_FILE_NAME } }
#define{{C_FILE_NAME } }

#define{{DEF_PREFIX } } SAMPLERATE{{SAMPLERATE } }
#define{{DEF_PREFIX } } BPM{{BPM } }

{ % LOOP PORTAL_COUNT % }
#define{{PORTAL_NAME_{ % INDEX % } } } { % INDEX % }
{ % END % }

#ifdef __cplusplus
extern "C" {
#endif
void __cdecl {
    { FUNC_PREFIX }
}
init();
void __cdecl {
    { FUNC_PREFIX }
}
cleanup();
void __cdecl {
    { FUNC_PREFIX }
}
generate();

void *__cdecl {
    { FUNC_PREFIX }
}
get_portal(uint32_t id);
#ifdef __cplusplus
}
#endif

#endif
