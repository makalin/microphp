# micro-PHP Compiler (microphpc)
set(COMPILER_SOURCES
    main.c
    compiler.c
)

# Create compiler executable
add_executable(microphpc ${COMPILER_SOURCES})

# Link with core library
target_link_libraries(microphpc microphp_core)

# Set include directories
target_include_directories(microphpc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set C standard
set_target_properties(microphpc PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

# Install rules
install(TARGETS microphpc
    RUNTIME DESTINATION bin
)
