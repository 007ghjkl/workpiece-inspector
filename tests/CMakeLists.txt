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

qt_add_executable(configuration-tests
    configuration_test.cpp
)

target_link_libraries(configuration-tests
    PRIVATE
        workpiece-configuration
        Qt6::Test
)

add_test(NAME configuration COMMAND configuration-tests)

qt_add_executable(simulated-image-source-tests
    simulated_image_source_test.cpp
)

target_link_libraries(simulated-image-source-tests
    PRIVATE
        workpiece-vision
        Qt6::Test
)

add_test(NAME simulated-image-source COMMAND simulated-image-source-tests)

qt_add_executable(network-camera-source-tests
    network_camera_source_test.cpp
)

target_link_libraries(network-camera-source-tests
    PRIVATE
        workpiece-vision
        Qt6::Test
)

add_test(NAME network-camera-source COMMAND network-camera-source-tests)

qt_add_executable(rule-based-inspector-tests
    rule_based_inspector_test.cpp
)

target_link_libraries(rule-based-inspector-tests
    PRIVATE
        workpiece-vision
        Qt6::Test
)

add_test(NAME rule-based-inspector COMMAND rule-based-inspector-tests)
