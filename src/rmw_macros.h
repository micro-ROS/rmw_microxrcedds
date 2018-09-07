// Internal defines
#ifndef RMW_MICRORTPS_DEFAULT_MEM_SIZE
#define RMW_MICRORTPS_DEFAULT_MEM_SIZE 5
#endif

#ifndef RMW_MICRORTPS_INTERNAL_NODE_SIZE
#define RMW_MICRORTPS_INTERNAL_NODE_SIZE RMW_MICRORTPS_DEFAULT_MEM_SIZE
#endif

#ifndef RMW_MICRORTPS_SUBSCRIBER_RAW_BUFFER_SIZE
#define RMW_MICRORTPS_SUBSCRIBER_RAW_BUFFER_SIZE 250
#endif


#ifndef RMW_MICRORTPS_INTERNAL_WAIT_SET_SIZE
#define RMW_MICRORTPS_INTERNAL_WAIT_SET_SIZE RMW_MICRORTPS_DEFAULT_MEM_SIZE
#endif

#ifndef RMW_MICRORTPS_INTERNAL_SUBSCRIBER_INFO_SIZE
#define RMW_MICRORTPS_INTERNAL_SUBSCRIBER_INFO_SIZE RMW_MICRORTPS_DEFAULT_MEM_SIZE
#endif

#define RMW_MICRORTPS_UNUSED_STACK_NAME(VarName) Unused_##VarName##_Stack

#define RMW_MICRORTPS_MEM_NAME(VarName) VarName##_Mem

#define RMW_MICRORTPS_MEM_SIZE(VarName) RMW_MICRORTPS_##VarName##_SIZE

#define RMW_MICRORTPS_DECLARE_INTERNAL_MEMORY_STRUCT(StructDef, MemberType, MemberName) \
    typedef struct StructDef \
    { \
        MemberType MemberName; \
        struct StructDef* Next; \
    } StructDef; \

#define RMW_MICRORTPS_DECLARE_INTENAL_MEMORY(StructDef, VarName) \
    static StructDef RMW_MICRORTPS_MEM_NAME(VarName)[RMW_MICRORTPS_MEM_SIZE(VarName)]; \
    static StructDef* RMW_MICRORTPS_UNUSED_STACK_NAME(VarName) = NULL;

#define RMW_MICRORTPS_INIT_INTERNAL_MEM(VarName) \
    RMW_MICRORTPS_UNUSED_STACK_NAME(VarName) = RMW_MICRORTPS_MEM_NAME(VarName); \
    for (unsigned i = 0; i < RMW_MICRORTPS_MEM_SIZE(VarName) - 1; i++) \
    { \
        RMW_MICRORTPS_MEM_NAME(VarName)[i].Next = &RMW_MICRORTPS_MEM_NAME(VarName)[i + 1]; \
    } \
    RMW_MICRORTPS_MEM_NAME(VarName)[RMW_MICRORTPS_MEM_SIZE(VarName) - 1].Next = NULL;

#define RMW_MICRORTPS_EXTRACT_FROM_INTERNAL_MEM(VarName, ExtractedPointer) \
    ExtractedPointer                         = RMW_MICRORTPS_UNUSED_STACK_NAME(VarName); \
    RMW_MICRORTPS_UNUSED_STACK_NAME(VarName) = RMW_MICRORTPS_UNUSED_STACK_NAME(VarName)->Next; \
    ExtractedPointer->Next                   = NULL;

#define RMW_MICRORTPS_RETURN_TO_INTERNAL_MEM(VarName, RetunedPointer) \
    memset(RetunedPointer, 0x00, sizeof(RMW_MICRORTPS_MEM_NAME(VarName)[0])); \
    RetunedPointer->Next                     = RMW_MICRORTPS_UNUSED_STACK_NAME(VarName); \
    RMW_MICRORTPS_UNUSED_STACK_NAME(VarName) = RetunedPointer;

#define RMW_MICRORTPS_CHECK_AVAILABLE(VarName) ((RMW_MICRORTPS_UNUSED_STACK_NAME(VarName) != NULL) ? true : false)