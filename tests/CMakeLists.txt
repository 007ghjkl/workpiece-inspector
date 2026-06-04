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

qt_add_executable(motion-simulation-tests
    motion_simulation_test.cpp
)

target_link_libraries(motion-simulation-tests
    PRIVATE
        workpiece-motion
        Qt6::Test
)

add_test(NAME motion-simulation COMMAND motion-simulation-tests)

qt_add_executable(communication-simulation-tests
    communication_simulation_test.cpp
)

target_link_libraries(communication-simulation-tests
    PRIVATE
        workpiece-communication
        Qt6::Test
)

add_test(NAME communication-simulation COMMAND communication-simulation-tests)

qt_add_executable(persistence-tests
    persistence_test.cpp
)

target_link_libraries(persistence-tests
    PRIVATE
        workpiece-persistence
        Qt6::Test
)

add_test(NAME persistence COMMAND persistence-tests)

qt_add_executable(image-storage-tests
    image_storage_test.cpp
)

target_link_libraries(image-storage-tests
    PRIVATE
        workpiece-persistence
        workpiece-storage
        Qt6::Test
)

add_test(NAME image-storage COMMAND image-storage-tests)

qt_add_executable(workflow-controller-tests
    workflow_controller_test.cpp
)

target_link_libraries(workflow-controller-tests
    PRIVATE
        workpiece-workflow
        Qt6::Test
)

add_test(NAME workflow-controller COMMAND workflow-controller-tests)
