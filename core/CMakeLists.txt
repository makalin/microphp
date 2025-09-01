# Core micro-PHP library
set(CORE_SOURCES
    vm.c
    zval.c
)

# Create core library
add_library(microphp_core STATIC ${CORE_SOURCES})

# Set include directories
target_include_directories(microphp_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set compile definitions based on options
target_compile_definitions(microphp_core PRIVATE
    $<$<BOOL:${MICROPHP_EXCEPTIONS}>:MICROPHP_EXCEPTIONS>
    $<$<BOOL:${MICROPHP_FLOAT64}>:MICROPHP_FLOAT64>
    $<$<BOOL:${MICROPHP_STDIO}>:MICROPHP_STDIO>
    $<$<BOOL:${MICROPHP_NET}>:MICROPHP_NET>
    $<$<BOOL:${MICROPHP_KVSTORE}>:MICROPHP_KVSTORE>
)

# Set memory configuration
target_compile_definitions(microphp_core PRIVATE
    MICROPHP_STR_ARENA_KB=${MICROPHP_STR_ARENA_KB}
    MICROPHP_ARRAY_ARENA_KB=${MICROPHP_ARRAY_ARENA_KB}
    MICROPHP_STACK_KB=${MICROPHP_STACK_KB}
    MICROPHP_TASKS_MAX=${MICROPHP_TASKS_MAX}
)

# Set C standard
set_target_properties(microphp_core PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

# Install rules
install(TARGETS microphp_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES microphp.h
    DESTINATION include/microphp
)
