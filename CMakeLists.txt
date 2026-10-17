cmake_minimum_required(VERSION 3.20)
project(HP6Reconstructed LANGUAGES CXX)

add_library(hp6_reconstructed STATIC
    src/archive/Entry.cpp
    src/archive/Header.cpp
    src/math/Scalar.cpp
    src/math/Vector.cpp
    src/math/VectorWithGlobal.cpp
    src/math/Transform.cpp
    src/math/FloatAccess.cpp
    src/stream/Metadata.cpp
    src/projection/RecordFields.cpp
    src/projection/RecordFlags.cpp
    src/projection/FormatSize.cpp
    src/projection/RecordDecode.cpp
    src/projection/AllocationDispatch.cpp
    src/projection/ChainClone.cpp
    src/projection/CollectionSizing.cpp
    src/projection/RecordLookup.cpp
    src/projection/RecordChainEdit.cpp
    src/platform/StartupSupport.cpp
    src/platform/GameWindow.cpp
    src/platform/Sync.cpp
    src/platform/ThreadPriority.cpp
    src/platform/TimerWheel.cpp
    src/platform/ServiceLifecycle.cpp
    src/platform/ServiceLifecycleExtra.cpp
    src/platform/HandleRegistry.cpp
    src/runtime/PoolAccess.cpp
    src/runtime/PoolList.cpp
    src/runtime/PoolPath.cpp
    src/runtime/PoolWait.cpp
    src/runtime/ListQueue.cpp
    src/runtime/WrapperStubs.cpp
    src/runtime/VolumeRegistry.cpp
)
target_include_directories(hp6_reconstructed PUBLIC src)
target_compile_features(hp6_reconstructed PUBLIC cxx_std_20)
if(WIN32)
    target_link_libraries(hp6_reconstructed PUBLIC user32 gdi32)
endif()

# The original target is x86. This library can also be compiled on x64 for
# source checks, but an x64 build is not an ABI-equivalence claim.
if(MSVC)
    target_compile_options(hp6_reconstructed PRIVATE /W4)
else()
    target_compile_options(hp6_reconstructed PRIVATE -Wall -Wextra)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(hp6_archive_header_smoke tests/archive_header_smoke.cpp)
    target_link_libraries(hp6_archive_header_smoke PRIVATE hp6_reconstructed)
    add_test(NAME archive_header_smoke COMMAND hp6_archive_header_smoke)
    add_executable(hp6_archive_entry_smoke tests/archive_entry_smoke.cpp)
    target_link_libraries(hp6_archive_entry_smoke PRIVATE hp6_reconstructed)
    add_test(NAME archive_entry_smoke COMMAND hp6_archive_entry_smoke)
    add_executable(hp6_pool_access_smoke tests/pool_access_smoke.cpp)
    target_link_libraries(hp6_pool_access_smoke PRIVATE hp6_reconstructed)
    add_test(NAME pool_access_smoke COMMAND hp6_pool_access_smoke)
    add_executable(hp6_pool_list_smoke tests/pool_list_smoke.cpp)
    target_link_libraries(hp6_pool_list_smoke PRIVATE hp6_reconstructed)
    add_test(NAME pool_list_smoke COMMAND hp6_pool_list_smoke)
    add_executable(hp6_pool_sync_smoke tests/pool_sync_smoke.cpp)
    target_link_libraries(hp6_pool_sync_smoke PRIVATE hp6_reconstructed)
    add_test(NAME pool_sync_smoke COMMAND hp6_pool_sync_smoke)
    add_executable(hp6_handle_registry_smoke tests/handle_registry_smoke.cpp)
    target_link_libraries(hp6_handle_registry_smoke PRIVATE hp6_reconstructed)
    add_test(NAME handle_registry_smoke COMMAND hp6_handle_registry_smoke)
    add_executable(hp6_service_cleanup_smoke tests/service_cleanup_smoke.cpp)
    target_link_libraries(hp6_service_cleanup_smoke PRIVATE hp6_reconstructed)
    add_test(NAME service_cleanup_smoke COMMAND hp6_service_cleanup_smoke)
    add_executable(hp6_engine_boot_smoke tests/engine_boot_smoke.cpp)
    target_link_libraries(hp6_engine_boot_smoke PRIVATE hp6_reconstructed)
    add_test(NAME engine_boot_smoke COMMAND hp6_engine_boot_smoke)
    add_executable(hp6_pool_wait_smoke tests/pool_wait_smoke.cpp)
    target_link_libraries(hp6_pool_wait_smoke PRIVATE hp6_reconstructed)
    add_test(NAME pool_wait_smoke COMMAND hp6_pool_wait_smoke)
    add_executable(hp6_volume_registry_smoke tests/volume_registry_smoke.cpp)
    target_link_libraries(hp6_volume_registry_smoke PRIVATE hp6_reconstructed)
    add_test(NAME volume_registry_smoke COMMAND hp6_volume_registry_smoke)
    add_executable(hp6_service_lifecycle_smoke tests/service_lifecycle_smoke.cpp)
    target_link_libraries(hp6_service_lifecycle_smoke PRIVATE hp6_reconstructed)
    add_test(NAME service_lifecycle_smoke COMMAND hp6_service_lifecycle_smoke)
    add_executable(hp6_core_smoke tests/core_smoke.cpp)
    target_link_libraries(hp6_core_smoke PRIVATE hp6_reconstructed)
    add_test(NAME core_smoke COMMAND hp6_core_smoke)
    add_executable(hp6_projection_flags_smoke tests/projection_flags_smoke.cpp)
    target_link_libraries(hp6_projection_flags_smoke PRIVATE hp6_reconstructed)
    add_test(NAME projection_flags_smoke COMMAND hp6_projection_flags_smoke)
    add_executable(hp6_projection_size_smoke tests/projection_size_smoke.cpp)
    target_link_libraries(hp6_projection_size_smoke PRIVATE hp6_reconstructed)
    add_test(NAME projection_size_smoke COMMAND hp6_projection_size_smoke)
    add_executable(hp6_projection_update_size_smoke tests/projection_update_size_smoke.cpp)
    target_link_libraries(hp6_projection_update_size_smoke PRIVATE hp6_reconstructed)
    add_test(NAME projection_update_size_smoke COMMAND hp6_projection_update_size_smoke)
    add_executable(hp6_projection_lookup_smoke tests/projection_lookup_smoke.cpp)
    target_link_libraries(hp6_projection_lookup_smoke PRIVATE hp6_reconstructed)
    add_test(NAME projection_lookup_smoke COMMAND hp6_projection_lookup_smoke)
    add_executable(hp6_projection_chain_edit_smoke tests/projection_chain_edit_smoke.cpp)
    target_link_libraries(hp6_projection_chain_edit_smoke PRIVATE hp6_reconstructed)
    add_test(NAME projection_chain_edit_smoke COMMAND hp6_projection_chain_edit_smoke)
    add_executable(hp6_projection_decode_smoke tests/projection_decode_smoke.cpp)
    target_link_libraries(hp6_projection_decode_smoke PRIVATE hp6_reconstructed)
    add_test(NAME projection_decode_smoke COMMAND hp6_projection_decode_smoke)
    add_executable(hp6_projection_create_smoke tests/projection_create_smoke.cpp)
    target_link_libraries(hp6_projection_create_smoke PRIVATE hp6_reconstructed)
    add_test(NAME projection_create_smoke COMMAND hp6_projection_create_smoke)
    add_executable(hp6_startup_support_smoke tests/startup_support_smoke.cpp)
    target_link_libraries(hp6_startup_support_smoke PRIVATE hp6_reconstructed)
    add_test(NAME startup_support_smoke COMMAND hp6_startup_support_smoke)
    if(WIN32)
        add_executable(hp6_game_window_smoke tests/game_window_smoke.cpp)
        target_link_libraries(hp6_game_window_smoke PRIVATE hp6_reconstructed)
        add_test(NAME game_window_smoke COMMAND hp6_game_window_smoke)
    endif()
endif()
