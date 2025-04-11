
add_executable(${CMAKE_PROJECT_NAME}
    src/main.cpp
    src/board.cpp
)


target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
    inc
    .
)


target_compile_options(${CMAKE_PROJECT_NAME}
    PRIVATE
    -Wpedantic
    -Wconversion
)


target_link_libraries(${CMAKE_PROJECT_NAME}
    project_options
    #  modm_arch_options
    modm_options
    modm_warnings
    modm
)

modm_targets_create(${CMAKE_PROJECT_NAME})