qt_add_executable(workpiece-inspector-tests
    smoke_test.cpp
)

target_link_libraries(workpiece-inspector-tests
    PRIVATE
        Qt6::Test
)

add_test(NAME smoke COMMAND workpiece-inspector-tests)
