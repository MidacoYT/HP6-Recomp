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
    src/stream/ChannelRegistry.cpp
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
    src/platform/Diagnostics.cpp
    src/platform/RegistryConfig.cpp
    src/platform/StorageInit.cpp
    src/platform/MessageLoop.cpp
    src/platform/EngineSubsystems.cpp
    src/platform/WinMain.cpp
    src/render/RenderRegionManager.cpp
    src/render/ShaderManager.cpp
    src/render/DisplayMode.cpp
    src/render/DeviceReset.cpp
    src/render/DeviceLostHandler.cpp
    src/render/DeviceCreate.cpp
    src/render/RenderHeap.cpp
    src/render/RenderStateCache.cpp
    src/render/RenderTargetSetup.cpp
    src/render/ScreenOverlayRender.cpp
    src/render/SceneBatchRenderer.cpp
    src/render/SubmitRenderCommand.cpp
    src/runtime/PoolAccess.cpp
    src/runtime/PoolList.cpp
    src/runtime/PoolPath.cpp
    src/runtime/PoolWait.cpp
    src/runtime/FileOps.cpp
    src/runtime/ListQueue.cpp
    src/runtime/WrapperStubs.cpp
    src/runtime/VolumeRegistry.cpp
    src/runtime/ScratchBuffers.cpp
    src/runtime/RecordDescriptor.cpp
    src/runtime/FrameClock.cpp
    src/presentation/TaskSystem.cpp
    src/presentation/CommandDispatch.cpp
    src/presentation/GuiTextTable.cpp
    src/presentation/ScreenTasks.cpp
    src/presentation/EngineBootstrap.cpp
    src/presentation/ChapterFlow.cpp
    src/presentation/ChapterLoadStateMachine.cpp
    src/presentation/BootupFlow.cpp
)
target_include_directories(hp6_reconstructed PUBLIC src)
target_compile_features(hp6_reconstructed PUBLIC cxx_std_20)
if(WIN32)
    target_link_libraries(hp6_reconstructed PUBLIC user32 gdi32 winmm)
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
    add_executable(hp6_stream_priority_smoke tests/stream_priority_smoke.cpp)
    target_link_libraries(hp6_stream_priority_smoke PRIVATE hp6_reconstructed)
    add_test(NAME stream_priority_smoke COMMAND hp6_stream_priority_smoke)
    add_executable(hp6_scratch_buffers_smoke tests/scratch_buffers_smoke.cpp)
    target_link_libraries(hp6_scratch_buffers_smoke PRIVATE hp6_reconstructed)
    add_test(NAME scratch_buffers_smoke COMMAND hp6_scratch_buffers_smoke)
    add_executable(hp6_render_smoke tests/render_smoke.cpp)
    target_link_libraries(hp6_render_smoke PRIVATE hp6_reconstructed)
    add_test(NAME render_smoke COMMAND hp6_render_smoke)
    add_executable(hp6_device_reset_smoke tests/device_reset_smoke.cpp)
    target_link_libraries(hp6_device_reset_smoke PRIVATE hp6_reconstructed)
    add_test(NAME device_reset_smoke COMMAND hp6_device_reset_smoke)
    add_executable(hp6_device_lost_handler_smoke tests/device_lost_handler_smoke.cpp)
    target_link_libraries(hp6_device_lost_handler_smoke PRIVATE hp6_reconstructed)
    add_test(NAME device_lost_handler_smoke COMMAND hp6_device_lost_handler_smoke)
    add_executable(hp6_message_loop_smoke tests/message_loop_smoke.cpp)
    target_link_libraries(hp6_message_loop_smoke PRIVATE hp6_reconstructed)
    add_test(NAME message_loop_smoke COMMAND hp6_message_loop_smoke)
    add_executable(hp6_winmain_smoke tests/winmain_smoke.cpp)
    target_link_libraries(hp6_winmain_smoke PRIVATE hp6_reconstructed)
    add_test(NAME winmain_smoke COMMAND hp6_winmain_smoke)
    add_executable(hp6_screen_overlay_render_smoke tests/screen_overlay_render_smoke.cpp)
    target_link_libraries(hp6_screen_overlay_render_smoke PRIVATE hp6_reconstructed)
    add_test(NAME screen_overlay_render_smoke COMMAND hp6_screen_overlay_render_smoke)
    add_executable(hp6_scene_batch_renderer_smoke tests/scene_batch_renderer_smoke.cpp)
    target_link_libraries(hp6_scene_batch_renderer_smoke PRIVATE hp6_reconstructed)
    add_test(NAME scene_batch_renderer_smoke COMMAND hp6_scene_batch_renderer_smoke)

    add_executable(hp6_submit_render_command_smoke tests/submit_render_command_smoke.cpp)
    target_link_libraries(hp6_submit_render_command_smoke PRIVATE hp6_reconstructed)
    add_test(NAME submit_render_command_smoke COMMAND hp6_submit_render_command_smoke)
    add_executable(hp6_device_create_smoke tests/device_create_smoke.cpp)
    target_link_libraries(hp6_device_create_smoke PRIVATE hp6_reconstructed)
    add_test(NAME device_create_smoke COMMAND hp6_device_create_smoke)
    add_executable(hp6_render_state_cache_smoke tests/render_state_cache_smoke.cpp)
    target_link_libraries(hp6_render_state_cache_smoke PRIVATE hp6_reconstructed)
    add_test(NAME render_state_cache_smoke COMMAND hp6_render_state_cache_smoke)
    add_executable(hp6_record_descriptor_smoke tests/record_descriptor_smoke.cpp)
    target_link_libraries(hp6_record_descriptor_smoke PRIVATE hp6_reconstructed)
    add_test(NAME record_descriptor_smoke COMMAND hp6_record_descriptor_smoke)
    add_executable(hp6_task_system_smoke tests/task_system_smoke.cpp)
    target_link_libraries(hp6_task_system_smoke PRIVATE hp6_reconstructed)
    add_test(NAME task_system_smoke COMMAND hp6_task_system_smoke)
    add_executable(hp6_gui_text_table_smoke tests/gui_text_table_smoke.cpp)
    target_link_libraries(hp6_gui_text_table_smoke PRIVATE hp6_reconstructed)
    add_test(NAME gui_text_table_smoke COMMAND hp6_gui_text_table_smoke)
    add_executable(hp6_screen_tasks_smoke tests/screen_tasks_smoke.cpp)
    target_link_libraries(hp6_screen_tasks_smoke PRIVATE hp6_reconstructed)
    add_test(NAME screen_tasks_smoke COMMAND hp6_screen_tasks_smoke)
    add_executable(hp6_command_dispatch_smoke tests/command_dispatch_smoke.cpp)
    target_link_libraries(hp6_command_dispatch_smoke PRIVATE hp6_reconstructed)
    add_test(NAME command_dispatch_smoke COMMAND hp6_command_dispatch_smoke)
    add_executable(hp6_engine_bootstrap_smoke tests/engine_bootstrap_smoke.cpp)
    target_link_libraries(hp6_engine_bootstrap_smoke PRIVATE hp6_reconstructed)
    add_test(NAME engine_bootstrap_smoke COMMAND hp6_engine_bootstrap_smoke)
    add_executable(hp6_chapter_flow_smoke tests/chapter_flow_smoke.cpp)
    target_link_libraries(hp6_chapter_flow_smoke PRIVATE hp6_reconstructed)
    add_test(NAME chapter_flow_smoke COMMAND hp6_chapter_flow_smoke)
    add_executable(hp6_chapter_load_state_machine_smoke tests/chapter_load_state_machine_smoke.cpp)
    target_link_libraries(hp6_chapter_load_state_machine_smoke PRIVATE hp6_reconstructed)
    add_test(NAME chapter_load_state_machine_smoke COMMAND hp6_chapter_load_state_machine_smoke)
    add_executable(hp6_bootup_flow_smoke tests/bootup_flow_smoke.cpp)
    target_link_libraries(hp6_bootup_flow_smoke PRIVATE hp6_reconstructed)
    add_test(NAME bootup_flow_smoke COMMAND hp6_bootup_flow_smoke)
    if(WIN32)
        add_executable(hp6_game_window_smoke tests/game_window_smoke.cpp)
        target_link_libraries(hp6_game_window_smoke PRIVATE hp6_reconstructed)
        add_test(NAME game_window_smoke COMMAND hp6_game_window_smoke)
        add_executable(hp6_registry_config_smoke tests/registry_config_smoke.cpp)
        target_link_libraries(hp6_registry_config_smoke PRIVATE hp6_reconstructed)
        add_test(NAME registry_config_smoke COMMAND hp6_registry_config_smoke)
        add_executable(hp6_frame_clock_smoke tests/frame_clock_smoke.cpp)
        target_link_libraries(hp6_frame_clock_smoke PRIVATE hp6_reconstructed)
        add_test(NAME frame_clock_smoke COMMAND hp6_frame_clock_smoke)
        add_executable(hp6_storage_init_smoke tests/storage_init_smoke.cpp)
        target_link_libraries(hp6_storage_init_smoke PRIVATE hp6_reconstructed)
        add_test(NAME storage_init_smoke COMMAND hp6_storage_init_smoke)
    endif()
endif()
