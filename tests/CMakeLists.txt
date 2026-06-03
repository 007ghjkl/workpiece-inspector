qt_add_executable(workpiece-inspector-tests
    smoke_test.cpp
)

target_link_libraries(workpiece-inspector-tests
    PRIVATE
        Qt6::Test
)

add_test(NAME smoke COMMAND workpiece-inspector-tests)

qt_add_executable(domain-models-tests
    domain_models_test.cpp
)

target_link_libraries(domain-models-tests
    PRIVATE
        workpiece-core
        Qt6::Test
)

add_test(NAME domain-models COMMAND domain-models-tests)
