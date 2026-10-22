cmake_minimum_required(VERSION 3.20)
project(HP6Reconstructed LANGUAGES CXX)

add_library(hp6_reconstructed STATIC
    src/audio/PathVolume.cpp
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
    src/platform/SyncAccessors.cpp
    src/platform/ThreadPriority.cpp
    src/platform/TimerWheel.cpp
    src/platform/ServiceLifecycle.cpp
    src/platform/ServiceLifecycleExtra.cpp
    src/platform/HandleRegistry.cpp
    src/platform/ServiceVtableConstructors.cpp
    src/platform/ServiceHolderDestructor.cpp
    src/platform/ServiceHolderSecondary.cpp
    src/platform/ServiceHolderAllocateOptions.cpp
    src/platform/ServiceHolderAllocateNamed.cpp
    src/platform/ServiceHolderAllocateContext.cpp
    src/platform/ServiceHolderHeapInitialize.cpp
    src/platform/ServiceContextName.cpp
    src/platform/ServiceContextRegistry.cpp
    src/platform/ServicePrimaryHeapGrowth.cpp
    src/platform/NativeServiceHolderVtable.cpp
    src/platform/ServiceHolderCleanup.cpp
    src/platform/ServiceHeapLeafs.cpp
    src/platform/ServiceHeapLock.cpp
    src/platform/ServiceHeapNode.cpp
    src/platform/ServiceHeapCoalesce.cpp
    src/platform/ServiceHeapCoalesceGeneral.cpp
    src/platform/ServiceHeapInitialize.cpp
    src/platform/ServiceHeapConstruct.cpp
    src/platform/ServiceHeapDispatch.cpp
    src/platform/ServiceHeapEnableLock.cpp
    src/platform/ServiceHeapFree.cpp
    src/platform/ServiceHeapAllocationWrappers.cpp
    src/platform/ServiceHeapDebugCallbacks.cpp
    src/platform/ServiceHandleFree.cpp
    src/platform/ServiceHeapReleaseCheck.cpp
    src/platform/ServiceHeapTailBlock.cpp
    src/platform/ServiceHeapReleasePages.cpp
    src/platform/ServiceHeapTeardown.cpp
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
    src/runtime/VerifiedBinaryLeafs.cpp
    src/presentation/TaskSystem.cpp
    src/presentation/CommandDispatch.cpp
    src/presentation/NativeCommandLeafs.cpp
    src/presentation/GuiTextTable.cpp
    src/presentation/ScreenTasks.cpp
    src/presentation/NativeScreenRecordMethods.cpp
    src/presentation/LanguageSelectionStart.cpp
    src/presentation/EngineBootstrap.cpp
    src/presentation/EngineEventSnapshot.cpp
    src/presentation/EngineRenderGate.cpp
    src/presentation/EngineRenderGateTick.cpp
    src/presentation/EngineRenderGateSimulate.cpp
    src/presentation/EngineTickDispatch.cpp
    src/presentation/EngineMessageDelivery.cpp
    src/presentation/EngineListenerRecycle.cpp
    src/presentation/ChapterFlow.cpp
    src/presentation/ChapterLoadStateMachine.cpp
    src/presentation/BootupFlow.cpp
    src/presentation/TextLayout.cpp
    src/presentation/BitmapRenderQuad.cpp
    src/presentation/BitmapRenderSubmit.cpp
    src/presentation/PresentationRenderDispatch.cpp
    src/presentation/LanguageSelectAssets.cpp
    src/presentation/SasCurveEvaluator.cpp
    src/presentation/SasColorAnimation.cpp
    src/presentation/LanguageSkuConfig.cpp
    src/presentation/PresentationStringRender.cpp
    src/presentation/PresentationStringLayout.cpp
    src/presentation/PresentationStringGeometry.cpp
    src/presentation/PresentationFocusRegistry.cpp
    src/presentation/ShpiDxtDecode.cpp
    src/render/GeometryBatcher.cpp
    src/render/GeometryBatchFlush.cpp
)
target_include_directories(hp6_reconstructed PUBLIC src)
target_compile_features(hp6_reconstructed PUBLIC cxx_std_20)
if(WIN32)
    target_link_libraries(hp6_reconstructed PUBLIC user32 gdi32 winmm)
    add_library(hp6_native_render_aliases OBJECT
        src/render/DeviceCreate.cpp
        src/render/RenderStateCache.cpp)
    target_include_directories(hp6_native_render_aliases PRIVATE src)
    target_compile_features(hp6_native_render_aliases PRIVATE cxx_std_20)
    target_compile_definitions(hp6_native_render_aliases PRIVATE HP6_NATIVE_ARRAY_ALIASES)
    # The native entry point is available for incremental linking, but the
    # game's global storage and runtime service factories are not yet rebuilt.
    # Keep the normal build usable until those dependencies are supplied.
    add_executable(hp6 EXCLUDE_FROM_ALL WIN32
        src/platform/WinMain.cpp
        src/platform/NativeInitialGlobals.cpp
        src/platform/NativeImageArrays.cpp
        src/platform/NativeChapterTables.cpp
        src/platform/NativeDriveTable.cpp
        src/platform/NativeHeapOverrides.cpp
        src/platform/NativeArrayAliases.cpp
        src/platform/TimerCompletion.cpp
        src/platform/NativePurecallVtable.cpp
        src/platform/NativeSingleDestructorVtable.cpp
        src/platform/NativeServiceBaseVtable.cpp
    src/platform/NativeServiceFinalVtable.cpp
        src/presentation/NativeBootupTaskVtable.cpp
        src/presentation/NativeEngineRenderGateVtable.cpp
        src/presentation/NativeListenerPool.cpp
        $<TARGET_OBJECTS:hp6_native_render_aliases>)
    target_link_libraries(hp6 PRIVATE hp6_reconstructed d3d9 shell32)
    option(HP6_BUILD_LANGUAGE_INSPECTOR "Build the standalone asset inspection window" OFF)
    if(HP6_BUILD_LANGUAGE_INSPECTOR)
        add_executable(hp6_language_select_preview EXCLUDE_FROM_ALL WIN32
            src/presentation/LanguageSelectMenuApp.cpp)
        target_link_libraries(hp6_language_select_preview PRIVATE hp6_reconstructed shell32)
    endif()
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
    add_executable(hp6_language_sku_config_smoke tests/language_sku_config_smoke.cpp)
    target_link_libraries(hp6_language_sku_config_smoke PRIVATE hp6_reconstructed)
    add_test(NAME language_sku_config_smoke COMMAND hp6_language_sku_config_smoke)
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
    add_executable(hp6_verified_binary_leafs_smoke tests/verified_binary_leafs_smoke.cpp)
    target_link_libraries(hp6_verified_binary_leafs_smoke PRIVATE hp6_reconstructed)
    add_test(NAME verified_binary_leafs_smoke COMMAND hp6_verified_binary_leafs_smoke)
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
    add_executable(hp6_text_layout_smoke tests/text_layout_smoke.cpp)
    target_link_libraries(hp6_text_layout_smoke PRIVATE hp6_reconstructed)
    add_test(NAME text_layout_smoke COMMAND hp6_text_layout_smoke)
    add_executable(hp6_bitmap_render_quad_smoke tests/bitmap_render_quad_smoke.cpp)
    target_link_libraries(hp6_bitmap_render_quad_smoke PRIVATE hp6_reconstructed)
    add_test(NAME bitmap_render_quad_smoke COMMAND hp6_bitmap_render_quad_smoke)
    add_executable(hp6_presentation_render_dispatch_smoke tests/presentation_render_dispatch_smoke.cpp)
    target_link_libraries(hp6_presentation_render_dispatch_smoke PRIVATE hp6_reconstructed)
    add_test(NAME presentation_render_dispatch_smoke COMMAND hp6_presentation_render_dispatch_smoke)
    add_executable(hp6_presentation_string_render_smoke tests/presentation_string_render_smoke.cpp)
    target_link_libraries(hp6_presentation_string_render_smoke PRIVATE hp6_reconstructed)
    add_test(NAME presentation_string_render_smoke COMMAND hp6_presentation_string_render_smoke)
    add_executable(hp6_presentation_string_layout_smoke tests/presentation_string_layout_smoke.cpp)
    target_link_libraries(hp6_presentation_string_layout_smoke PRIVATE hp6_reconstructed)
    add_test(NAME presentation_string_layout_smoke COMMAND hp6_presentation_string_layout_smoke)
    if(EXISTS "${PROJECT_SOURCE_DIR}/../extract/chapters/languageselect.str"
       AND EXISTS "${PROJECT_SOURCE_DIR}/../extract/game.str")
        add_executable(hp6_language_select_assets_smoke tests/language_select_assets_smoke.cpp)
        target_link_libraries(hp6_language_select_assets_smoke PRIVATE hp6_reconstructed)
        add_test(NAME language_select_assets_smoke COMMAND hp6_language_select_assets_smoke
                 "${PROJECT_SOURCE_DIR}/../extract/chapters/languageselect.str"
                 "${PROJECT_SOURCE_DIR}/../extract/game.str")
    endif()
    if(WIN32)
        add_executable(hp6_timer_completion_smoke
            tests/timer_completion_smoke.cpp src/platform/TimerCompletion.cpp)
        target_compile_features(hp6_timer_completion_smoke PRIVATE cxx_std_20)
        add_test(NAME timer_completion_smoke COMMAND hp6_timer_completion_smoke)
        add_executable(hp6_game_window_smoke tests/game_window_smoke.cpp)
        target_link_libraries(hp6_game_window_smoke PRIVATE hp6_reconstructed)
        add_test(NAME game_window_smoke COMMAND hp6_game_window_smoke)
        add_executable(hp6_registry_config_smoke tests/registry_config_smoke.cpp)
        target_link_libraries(hp6_registry_config_smoke PRIVATE hp6_reconstructed)
        add_test(NAME registry_config_smoke COMMAND hp6_registry_config_smoke)
        add_executable(hp6_frame_clock_smoke tests/frame_clock_smoke.cpp)
        target_link_libraries(hp6_frame_clock_smoke PRIVATE hp6_reconstructed)
        add_test(NAME frame_clock_smoke COMMAND hp6_frame_clock_smoke)
        add_executable(hp6_engine_render_gate_smoke
            tests/engine_render_gate_smoke.cpp
            src/presentation/EngineRenderGate.cpp
            src/presentation/EngineRenderGateTick.cpp)
        target_include_directories(hp6_engine_render_gate_smoke PRIVATE src)
        target_compile_features(hp6_engine_render_gate_smoke PRIVATE cxx_std_20)
        add_test(NAME engine_render_gate_smoke COMMAND hp6_engine_render_gate_smoke)
        add_executable(hp6_engine_event_snapshot_smoke
            tests/engine_event_snapshot_smoke.cpp
            src/presentation/EngineEventSnapshot.cpp
            src/presentation/EngineRenderGateSimulate.cpp)
        target_include_directories(hp6_engine_event_snapshot_smoke PRIVATE src)
        target_compile_features(hp6_engine_event_snapshot_smoke PRIVATE cxx_std_20)
        add_test(NAME engine_event_snapshot_smoke COMMAND hp6_engine_event_snapshot_smoke)
        add_executable(hp6_engine_tick_dispatch_smoke
            tests/engine_tick_dispatch_smoke.cpp
            src/presentation/EngineTickDispatch.cpp)
        target_compile_features(hp6_engine_tick_dispatch_smoke PRIVATE cxx_std_20)
        add_test(NAME engine_tick_dispatch_smoke COMMAND hp6_engine_tick_dispatch_smoke)
        add_executable(hp6_engine_message_delivery_smoke
            tests/engine_message_delivery_smoke.cpp
            src/presentation/EngineMessageDelivery.cpp)
        target_compile_features(hp6_engine_message_delivery_smoke PRIVATE cxx_std_20)
        add_test(NAME engine_message_delivery_smoke COMMAND hp6_engine_message_delivery_smoke)
        add_executable(hp6_engine_listener_recycle_smoke
            tests/engine_listener_recycle_smoke.cpp
            src/presentation/EngineListenerRecycle.cpp)
        target_compile_features(hp6_engine_listener_recycle_smoke PRIVATE cxx_std_20)
        add_test(NAME engine_listener_recycle_smoke COMMAND hp6_engine_listener_recycle_smoke)
        add_executable(hp6_service_holder_destructor_smoke
            tests/service_holder_destructor_smoke.cpp
            src/platform/ServiceHolderDestructor.cpp
            src/platform/ServiceHolderSecondary.cpp)
        target_include_directories(hp6_service_holder_destructor_smoke PRIVATE src)
        target_compile_features(hp6_service_holder_destructor_smoke PRIVATE cxx_std_20)
        add_test(NAME service_holder_destructor_smoke COMMAND hp6_service_holder_destructor_smoke)
        add_executable(hp6_native_service_holder_vtable_smoke
            tests/native_service_holder_vtable_smoke.cpp
            src/platform/NativeServiceHolderVtable.cpp
            src/platform/ServiceHolderDestructor.cpp
            src/platform/ServiceHolderSecondary.cpp
            src/runtime/VerifiedBinaryLeafs.cpp)
        target_include_directories(hp6_native_service_holder_vtable_smoke PRIVATE src)
        target_compile_features(hp6_native_service_holder_vtable_smoke PRIVATE cxx_std_20)
        add_test(NAME native_service_holder_vtable_smoke COMMAND hp6_native_service_holder_vtable_smoke)
        add_executable(hp6_service_holder_allocate_options_smoke
            tests/service_holder_allocate_options_smoke.cpp
            src/platform/ServiceHolderAllocateOptions.cpp
            src/platform/ServiceHolderAllocateNamed.cpp
            src/platform/ServiceHolderAllocateContext.cpp
            src/platform/ServiceContextName.cpp)
        target_include_directories(hp6_service_holder_allocate_options_smoke PRIVATE src)
        target_compile_features(hp6_service_holder_allocate_options_smoke PRIVATE cxx_std_20)
        add_test(NAME service_holder_allocate_options_smoke COMMAND hp6_service_holder_allocate_options_smoke)
        add_executable(hp6_service_context_registry_smoke
            tests/service_context_registry_smoke.cpp
            src/platform/ServiceContextRegistry.cpp)
        target_compile_features(hp6_service_context_registry_smoke PRIVATE cxx_std_20)
        add_test(NAME service_context_registry_smoke COMMAND hp6_service_context_registry_smoke)
        add_executable(hp6_service_primary_heap_growth_smoke
            tests/service_primary_heap_growth_smoke.cpp
            src/platform/ServicePrimaryHeapGrowth.cpp)
        target_include_directories(hp6_service_primary_heap_growth_smoke PRIVATE src)
        target_compile_features(hp6_service_primary_heap_growth_smoke PRIVATE cxx_std_20)
        add_test(NAME service_primary_heap_growth_smoke COMMAND hp6_service_primary_heap_growth_smoke)
        add_executable(hp6_service_heap_allocation_wrappers_smoke
            tests/service_heap_allocation_wrappers_smoke.cpp
            src/platform/ServiceHeapAllocationWrappers.cpp)
        target_compile_features(hp6_service_heap_allocation_wrappers_smoke PRIVATE cxx_std_20)
        add_test(NAME service_heap_allocation_wrappers_smoke COMMAND hp6_service_heap_allocation_wrappers_smoke)
        add_executable(hp6_service_holder_cleanup_smoke
            tests/service_holder_cleanup_smoke.cpp
            src/platform/ServiceHolderCleanup.cpp)
        target_include_directories(hp6_service_holder_cleanup_smoke PRIVATE src)
        target_compile_features(hp6_service_holder_cleanup_smoke PRIVATE cxx_std_20)
        add_test(NAME service_holder_cleanup_smoke COMMAND hp6_service_holder_cleanup_smoke)
        add_executable(hp6_service_heap_teardown_smoke
            tests/service_heap_teardown_smoke.cpp
            src/platform/ServiceHeapTeardown.cpp
            src/platform/ServiceHeapLeafs.cpp)
        target_include_directories(hp6_service_heap_teardown_smoke PRIVATE src)
        target_compile_features(hp6_service_heap_teardown_smoke PRIVATE cxx_std_20)
        add_test(NAME service_heap_teardown_smoke COMMAND hp6_service_heap_teardown_smoke)
        add_executable(hp6_service_heap_initialize_smoke
            tests/service_heap_initialize_smoke.cpp
            src/platform/ServiceHeapInitialize.cpp
            src/platform/ServiceHeapEnableLock.cpp)
        target_include_directories(hp6_service_heap_initialize_smoke PRIVATE src)
        target_compile_features(hp6_service_heap_initialize_smoke PRIVATE cxx_std_20)
        add_test(NAME service_heap_initialize_smoke COMMAND hp6_service_heap_initialize_smoke)
        add_executable(hp6_service_heap_construct_smoke
            tests/service_heap_construct_smoke.cpp
            src/platform/ServiceHeapConstruct.cpp)
        target_include_directories(hp6_service_heap_construct_smoke PRIVATE src)
        target_compile_features(hp6_service_heap_construct_smoke PRIVATE cxx_std_20)
        add_test(NAME service_heap_construct_smoke COMMAND hp6_service_heap_construct_smoke)
        add_executable(hp6_service_heap_dispatch_smoke
            tests/service_heap_dispatch_smoke.cpp
            src/platform/ServiceHeapDispatch.cpp)
        target_compile_features(hp6_service_heap_dispatch_smoke PRIVATE cxx_std_20)
        add_test(NAME service_heap_dispatch_smoke COMMAND hp6_service_heap_dispatch_smoke)
        add_executable(hp6_native_service_final_vtable_smoke
            tests/native_service_final_vtable_smoke.cpp
            src/platform/NativeServiceFinalVtable.cpp)
        target_include_directories(hp6_native_service_final_vtable_smoke PRIVATE src)
        target_compile_features(hp6_native_service_final_vtable_smoke PRIVATE cxx_std_20)
        add_test(NAME native_service_final_vtable_smoke COMMAND hp6_native_service_final_vtable_smoke)
        add_executable(hp6_service_handle_free_smoke
            tests/service_handle_free_smoke.cpp
            src/platform/ServiceHandleFree.cpp)
        target_compile_features(hp6_service_handle_free_smoke PRIVATE cxx_std_20)
        add_test(NAME service_handle_free_smoke COMMAND hp6_service_handle_free_smoke)
        add_executable(hp6_native_screen_record_methods_smoke
            tests/native_screen_record_methods_smoke.cpp
            src/presentation/NativeScreenRecordMethods.cpp)
        target_include_directories(hp6_native_screen_record_methods_smoke PRIVATE src)
        target_compile_features(hp6_native_screen_record_methods_smoke PRIVATE cxx_std_20)
        add_test(NAME native_screen_record_methods_smoke COMMAND hp6_native_screen_record_methods_smoke)
        add_executable(hp6_service_heap_free_smoke
            tests/service_heap_free_smoke.cpp
            src/platform/ServiceHeapFree.cpp)
        target_compile_features(hp6_service_heap_free_smoke PRIVATE cxx_std_20)
        add_test(NAME service_heap_free_smoke COMMAND hp6_service_heap_free_smoke)
        add_executable(hp6_storage_init_smoke tests/storage_init_smoke.cpp)
        target_link_libraries(hp6_storage_init_smoke PRIVATE hp6_reconstructed)
        add_test(NAME storage_init_smoke COMMAND hp6_storage_init_smoke)
    endif()
endif()
