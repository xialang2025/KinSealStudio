cmake_minimum_required(VERSION 3.25)

project(KinSealStudioLite VERSION 0.10.1 LANGUAGES CXX) #20261111-1 change: Publish the native main-window shell independently.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.12 REQUIRED COMPONENTS Widgets Svg) #20261109-0 change

#20261109-0 add: Link the large SVG catalog as a resource object instead of compiling megabytes of C++ literals.
qt_add_big_resources(KINSEAL_GALLERY_RESOURCES system_gallery.qrc)

qt_add_executable(KinSealStudioLite WIN32
    src/main.cpp
    src/main_window.cpp
    src/main_window.h
    src/main_window_workflow.cpp #20261110-7 add: Share command state and whole-project transactions.
    src/native_shell_ui.cpp #20261111-1 add: Reproduce the original desktop shell using retained native icons and layout.
    native_shell.qrc
    src/stage3_project_edits.h #20261110-8 add
    src/stage3_project_edits.cpp
    src/stage3_resources.h
    src/stage3_resources.cpp
    src/control_rendering.h
    src/control_rendering.cpp
    src/stage2_project_dialogs.h
    src/stage2_project_dialogs.cpp
    src/stage2_global_tools.h
    src/stage2_global_tools.cpp
    src/stage2_system_settings.h #20261110-7 add: Keep recovered editor-only parameters behind the existing device gate.
    src/stage2_system_settings.cpp
    src/project_model.cpp
    src/project_model.h
    src/project_capabilities.h #20261110-5 add
    src/project_capabilities.cpp
    src/plc_address.cpp #20261109-0 add
    src/plc_address.h
    src/device_address_compiler.cpp #20261109-0 add
    src/device_address_compiler.h
    src/device_scan_table.cpp #20261109-0 add
    src/device_scan_table.h
    src/plc_address_dialog.cpp
    src/plc_address_dialog.h
    plc_catalog.qrc
    src/native_test_project.cpp
    src/native_test_project.h
    src/native_project_codec.h #20261109-0 add
    src/native_project_codec.cpp
    src/independent_downloader.cpp #20261108-10 add
    src/independent_downloader.h
    src/device_transfer_plan.cpp #20261109-0 add
    src/device_transfer_plan.h
    src/text_payload_compiler.cpp #20261108-11 add
    src/text_payload_compiler.h
    src/static_text_properties.cpp #20261108-14 add
    src/static_text_properties.h
    src/static_text_renderer.cpp
    src/static_text_renderer.h
    src/static_text_dialog.cpp
    src/static_text_dialog.h
    src/static_text_actions.cpp
    src/stage4_control_actions.cpp #20261112-0 add: Each new control retains an independent typed model and dialog.
    src/message_display_properties.h
    src/message_display_properties.cpp
    src/message_display_dialog.h
    src/message_display_dialog.cpp
    src/scale_properties.h
    src/scale_properties.cpp
    src/scale_dialog.h
    src/scale_dialog.cpp
    src/qr_code_properties.h #20261112-0 add
    src/qr_code_properties.cpp
    src/qr_code_dialog.h
    src/qr_code_dialog.cpp
    src/gif_display_properties.h #20261112-0 add: Keep original animations independent of normalized pictures.
    src/number_display_properties.h #20261112-0 add
    src/ascii_display_properties.h #20261112-0 add
    src/ascii_display_properties.cpp
    src/time_display_properties.h #20261112-0 add
    src/time_display_properties.cpp
    src/screen_button_properties.h #20261112-0 add
    src/screen_button_properties.cpp
    src/screen_button_dialog.cpp
    src/multi_state_button_properties.h #20261112-0 add
    src/multi_state_button_properties.cpp
    src/multi_state_button_dialog.cpp
    src/multi_function_button_properties.h #20261112-0 add
    src/multi_function_button_properties.cpp
    src/multi_function_button_dialog.cpp
    src/dropdown_properties.h #20261112-0 add
    src/dropdown_properties.cpp
    src/dropdown_dialog.h
    src/dropdown_dialog.cpp
    src/flow_block_properties.h #20261112-0 add
    src/flow_block_properties.cpp
    src/flow_block_dialog.h
    src/flow_block_dialog.cpp
    src/slider_properties.h #20261112-0 add
    src/slider_properties.cpp
    src/slider_dialog.h
    src/slider_dialog.cpp
    src/wheel_properties.h #20261112-0 add
    src/wheel_properties.cpp
    src/wheel_dialog.h
    src/wheel_dialog.cpp
    src/move_track_properties.h #20261112-0 add
    src/move_track_properties.cpp
    src/move_track_dialog.h
    src/move_track_dialog.cpp
    src/data_group_properties.h #20261112-1 add
    src/data_group_properties.cpp
    src/data_group_dialog.h
    src/data_group_dialog.cpp
    src/alarm_bar_properties.h #20261113-2 add
    src/alarm_bar_properties.cpp
    src/alarm_bar_dialog.h
    src/alarm_bar_dialog.cpp
    src/alarm_registration_properties.h
    src/alarm_registration_properties.cpp
    src/alarm_registration_dialog.h
    src/alarm_registration_dialog.cpp
    src/xy_curve_properties.h #20261112-1 add
    src/xy_curve_properties.cpp
    src/xy_curve_dialog.h
    src/xy_curve_dialog.cpp
    src/history_trend_properties.h #20261112-1 add
    src/history_trend_properties.cpp
    src/history_trend_dialog.h
    src/history_trend_dialog.cpp
    src/data_collection_properties.h
    src/data_collection_properties.cpp
    src/data_collection_dialog.h
    src/data_collection_dialog.cpp
    src/trend_properties.h #20261112-1 add
    src/trend_properties.cpp
    src/trend_dialog.h
    src/trend_dialog.cpp
    src/chart_properties.h #20261112-1 add
    src/chart_properties.cpp
    src/chart_dialog.h
    src/chart_dialog.cpp
    src/number_display_properties.cpp
    src/number_display_dialog.h
    src/number_display_dialog.cpp
    src/gif_display_properties.cpp
    src/gif_display_dialog.h
    src/gif_display_dialog.cpp
    src/animation_resources.h
    src/animation_resources.cpp
    src/image_display_properties.h #20261112-0 add
    src/image_display_properties.cpp
    src/image_display_dialog.h
    src/image_display_dialog.cpp
    third_party/qrcodegen/qrcodegen.cpp
    src/canvas_editing.cpp
    src/bit_button_properties.h #20261108-15 add
    src/bit_button_properties.cpp
    src/word_button_properties.h #20261109-6 add
    src/word_button_properties.cpp
    src/lamp_properties.h #20261109-8 add
    src/lamp_properties.cpp
    src/function_button_properties.h #20261110-0 add
    src/function_button_properties.cpp
    src/bit_button_payload_compiler.h #20261109-0 add
    src/bit_button_payload_compiler.cpp
    src/native_bit_button_control.h #20261109-0 add
    src/native_bit_button_control.cpp
    src/native_word_button_control.h #20261109-6 add
    src/native_word_button_control.cpp
    src/native_lamp_control.h #20261109-8 add
    src/native_lamp_control.cpp
    src/native_function_button_control.h #20261110-0 add
    src/native_function_button_control.cpp
    src/native_static_text_control.cpp #20261109-0 add
    src/native_static_text_control.h
    src/static_text_payload_compiler.cpp
    src/static_text_payload_compiler.h
    src/device_macro_compiler.h #20261109-0 add
    src/device_macro_compiler.cpp
    src/bit_button_renderer.h
    src/bit_button_renderer.cpp
    src/kvg_library.cpp #20261109-0 add
    src/kvg_library.h
    src/bit_button_dialog.h
    src/bit_button_dialog.cpp
    src/hmi_property_widgets.h
    src/hmi_property_widgets.cpp
    third_party/lzokay/lzokay.cpp #20261108-11 add: MIT-licensed LZO1X encoder.
    src/hmi_canvas.cpp
    src/hmi_canvas.h
    native_test_template.qrc
    ${KINSEAL_GALLERY_RESOURCES}
)

target_link_libraries(KinSealStudioLite PRIVATE
    Qt6::Widgets
    Qt6::Svg #20261109-0 add
)

target_include_directories(KinSealStudioLite PRIVATE third_party/lzokay) #20261108-11 add

if(WIN32)
    target_link_libraries(KinSealStudioLite PRIVATE setupapi)
endif()

#20261108-14 add: Run Qt event tests offscreen; do not ship the Qt Test runtime in the editor package.
find_package(Qt6 REQUIRED COMPONENTS Test)
get_target_property(KINSEAL_TEST_SOURCES KinSealStudioLite SOURCES)
list(REMOVE_ITEM KINSEAL_TEST_SOURCES src/main.cpp)
#20261110-5 add: Exercise schema migration and the actual download backend without accessing USB.
qt_add_executable(KinSealProjectModelTests ${KINSEAL_TEST_SOURCES} tests/project_model_stage1_tests.cpp)
target_include_directories(KinSealProjectModelTests PRIVATE src third_party/lzokay)
target_link_libraries(KinSealProjectModelTests PRIVATE Qt6::Widgets Qt6::Svg Qt6::Test setupapi)
target_compile_definitions(KinSealProjectModelTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

qt_add_executable(KinSealStage1BackendTests ${KINSEAL_TEST_SOURCES} tests/stage1_backend_tests.cpp)
target_include_directories(KinSealStage1BackendTests PRIVATE src third_party/lzokay)
target_link_libraries(KinSealStage1BackendTests PRIVATE Qt6::Widgets Qt6::Svg Qt6::Test setupapi)
target_compile_definitions(KinSealStage1BackendTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261110-7 add: Verify real configuration dialogs and cross-page batch edits without USB access.
qt_add_executable(KinSealStage2DialogsTests ${KINSEAL_TEST_SOURCES} tests/stage2_project_dialogs_tests.cpp)
qt_add_executable(KinSealStage2GlobalToolsTests ${KINSEAL_TEST_SOURCES} tests/stage2_global_tools_tests.cpp)
qt_add_executable(KinSealStage2WorkflowTests ${KINSEAL_TEST_SOURCES} tests/stage2_workflow_tests.cpp)
foreach(KINSEAL_STAGE2_TARGET KinSealStage2DialogsTests KinSealStage2GlobalToolsTests KinSealStage2WorkflowTests)
    target_include_directories(${KINSEAL_STAGE2_TARGET} PRIVATE src third_party/lzokay)
    target_link_libraries(${KINSEAL_STAGE2_TARGET} PRIVATE Qt6::Widgets Qt6::Svg Qt6::Test setupapi)
    target_compile_definitions(${KINSEAL_STAGE2_TARGET} PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
endforeach()

#20261110-8 add: Exercise canvas gestures and complete host transactions against production implementations.
qt_add_executable(KinSealStage3CanvasTests ${KINSEAL_TEST_SOURCES} tests/stage3_canvas_tests.cpp)
qt_add_executable(KinSealStage3ProjectEditsTests ${KINSEAL_TEST_SOURCES} tests/stage3_project_edits_tests.cpp)
qt_add_executable(KinSealStage3IntegrationTests ${KINSEAL_TEST_SOURCES} tests/stage3_integration_tests.cpp)
foreach(KINSEAL_STAGE3_TARGET KinSealStage3CanvasTests KinSealStage3ProjectEditsTests KinSealStage3IntegrationTests)
    target_include_directories(${KINSEAL_STAGE3_TARGET} PRIVATE src third_party/lzokay)
    target_link_libraries(${KINSEAL_STAGE3_TARGET} PRIVATE Qt6::Widgets Qt6::Svg Qt6::Test setupapi)
    target_compile_definitions(${KINSEAL_STAGE3_TARGET} PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
endforeach()

qt_add_executable(KinSealStaticTextTests ${KINSEAL_TEST_SOURCES} tests/static_text_tests.cpp)
target_include_directories(KinSealStaticTextTests PRIVATE src third_party/lzokay)
target_link_libraries(KinSealStaticTextTests PRIVATE Qt6::Widgets Qt6::Svg Qt6::Test setupapi) #20261109-0 change
target_compile_definitions(KinSealStaticTextTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261112-0 add: Exercise the actual new-control dialogs, source gates and project transactions on a private desktop.
qt_add_executable(KinSealStage4ControlTests ${KINSEAL_TEST_SOURCES} tests/stage4_controls_tests.cpp)
target_include_directories(KinSealStage4ControlTests PRIVATE src third_party/lzokay)
target_link_libraries(KinSealStage4ControlTests PRIVATE Qt6::Widgets Qt6::Svg Qt6::Test setupapi)
target_compile_definitions(KinSealStage4ControlTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
target_sources(KinSealStage4ControlTests PRIVATE src/plc_native_catalog.cpp src/plc_native_catalog.h)

#20261109-6 add: Word dialog interaction tests share the actual application code and resource catalog.
qt_add_executable(KinSealWordButtonDialogTests ${KINSEAL_TEST_SOURCES} tests/word_button_dialog_tests.cpp)
target_include_directories(KinSealWordButtonDialogTests PRIVATE src third_party/lzokay)
target_link_libraries(KinSealWordButtonDialogTests PRIVATE Qt6::Widgets Qt6::Svg Qt6::Test setupapi)
target_compile_definitions(KinSealWordButtonDialogTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261109-8 add: Exercise lamp-only monitoring and every editable multi-state appearance.
qt_add_executable(KinSealLampDialogTests ${KINSEAL_TEST_SOURCES} tests/lamp_dialog_tests.cpp)
target_include_directories(KinSealLampDialogTests PRIVATE src third_party/lzokay)
target_link_libraries(KinSealLampDialogTests PRIVATE Qt6::Widgets Qt6::Svg Qt6::Test setupapi)
target_compile_definitions(KinSealLampDialogTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261110-0 add: Exercise all system-function selections and shared dialog transactions.
qt_add_executable(KinSealFunctionButtonDialogTests ${KINSEAL_TEST_SOURCES} tests/function_button_dialog_tests.cpp)
target_include_directories(KinSealFunctionButtonDialogTests PRIVATE src third_party/lzokay)
target_link_libraries(KinSealFunctionButtonDialogTests PRIVATE Qt6::Widgets Qt6::Svg Qt6::Test setupapi)
target_compile_definitions(KinSealFunctionButtonDialogTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

qt_add_executable(KinSealNativeFunctionControlTests
    src/native_function_button_control.cpp src/native_bit_button_control.cpp
    src/function_button_properties.cpp src/bit_button_properties.cpp src/static_text_properties.cpp
    tests/native_function_button_control_tests.cpp)
target_include_directories(KinSealNativeFunctionControlTests PRIVATE src)
target_link_libraries(KinSealNativeFunctionControlTests PRIVATE Qt6::Widgets Qt6::Test)
target_compile_definitions(KinSealNativeFunctionControlTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

qt_add_executable(KinSealNativeLampControlTests
    src/native_lamp_control.cpp src/native_bit_button_control.cpp
    src/lamp_properties.cpp src/word_button_properties.cpp src/bit_button_properties.cpp src/static_text_properties.cpp
    tests/native_lamp_control_tests.cpp)
target_include_directories(KinSealNativeLampControlTests PRIVATE src)
target_link_libraries(KinSealNativeLampControlTests PRIVATE Qt6::Widgets Qt6::Test)
target_compile_definitions(KinSealNativeLampControlTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261109-0 add: Original caption pixel masks use the Windows font backend without creating a window.
qt_add_executable(KinSealBitButtonRendererTests
    src/bit_button_renderer.cpp src/static_text_renderer.cpp src/kvg_library.cpp
    tests/bit_button_renderer_tests.cpp)
target_include_directories(KinSealBitButtonRendererTests PRIVATE src)
target_link_libraries(KinSealBitButtonRendererTests PRIVATE Qt6::Core Qt6::Gui Qt6::Svg Qt6::Test)
target_compile_definitions(KinSealBitButtonRendererTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261109-0 add: Verify recorded transfer ordering without opening a USB device or a window.
qt_add_executable(KinSealDeviceTransferTests
    src/device_transfer_plan.cpp src/device_transfer_plan.h tests/device_transfer_tests.cpp)
target_include_directories(KinSealDeviceTransferTests PRIVATE src)
target_link_libraries(KinSealDeviceTransferTests PRIVATE Qt6::Core Qt6::Test)

#20261109-0 add: Native record oracles exclude uninitialized bytes in the vendor writer.
qt_add_executable(KinSealNativeControlTests
    src/native_bit_button_control.cpp src/native_bit_button_control.h
    src/bit_button_properties.cpp src/static_text_properties.cpp
    tests/native_bit_button_control_tests.cpp)
target_include_directories(KinSealNativeControlTests PRIVATE src)
target_link_libraries(KinSealNativeControlTests PRIVATE Qt6::Widgets Qt6::Test)
target_compile_definitions(KinSealNativeControlTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261109-6 add: Word union records use independent native fixtures, widths and operation codes.
qt_add_executable(KinSealNativeWordControlTests
    src/native_word_button_control.cpp src/native_bit_button_control.cpp
    src/word_button_properties.cpp src/bit_button_properties.cpp src/static_text_properties.cpp
    tests/native_word_button_control_tests.cpp)
target_include_directories(KinSealNativeWordControlTests PRIVATE src)
target_link_libraries(KinSealNativeWordControlTests PRIVATE Qt6::Widgets Qt6::Test)
target_compile_definitions(KinSealNativeWordControlTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261109-0 add: Localized and permission-controlled text shares the observed type28 header.
qt_add_executable(KinSealNativeStaticControlTests src/native_static_text_control.cpp
    src/static_text_properties.cpp src/bit_button_properties.cpp tests/native_static_text_control_tests.cpp)
target_include_directories(KinSealNativeStaticControlTests PRIVATE src)
target_link_libraries(KinSealNativeStaticControlTests PRIVATE Qt6::Widgets Qt6::Test)
target_compile_definitions(KinSealNativeStaticControlTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261109-0 add: A supplied ARM SDK enables real C compilation checks; ordinary tests need no SDK.
qt_add_executable(KinSealDeviceMacroTests
    src/device_macro_compiler.cpp src/device_macro_compiler.h tests/device_macro_tests.cpp
    src/project_model.cpp src/native_test_project.cpp src/static_text_properties.cpp src/bit_button_properties.cpp)
target_sources(KinSealDeviceMacroTests PRIVATE src/plc_address.cpp src/native_project_codec.cpp plc_catalog.qrc) #20261109-0 change
target_include_directories(KinSealDeviceMacroTests PRIVATE src)
target_link_libraries(KinSealDeviceMacroTests PRIVATE Qt6::Widgets Qt6::Test)
target_compile_definitions(KinSealDeviceMacroTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261109-0 add: Catalog and address codec tests never load or execute a vendor driver.
qt_add_executable(KinSealPlcAddressTests src/plc_address.cpp src/plc_address.h tests/plc_address_tests.cpp plc_catalog.qrc)
target_include_directories(KinSealPlcAddressTests PRIVATE src)
target_link_libraries(KinSealPlcAddressTests PRIVATE Qt6::Core Qt6::Test)
target_compile_definitions(KinSealPlcAddressTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261109-0 add: Replay captured original codec results for every supported profile family.
qt_add_executable(KinSealPlcNativeCatalogTests src/plc_address.cpp src/plc_native_catalog.cpp
    tests/plc_native_catalog_tests.cpp plc_catalog.qrc)
target_include_directories(KinSealPlcNativeCatalogTests PRIVATE src)
target_link_libraries(KinSealPlcNativeCatalogTests PRIVATE Qt6::Core Qt6::Test)
target_compile_definitions(KinSealPlcNativeCatalogTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261109-0 add: Regress UI scan files and captured original Common sorter outputs offline.
qt_add_executable(KinSealDeviceScanTests src/device_scan_table.cpp tests/device_scan_table_tests.cpp)
target_include_directories(KinSealDeviceScanTests PRIVATE src)
target_link_libraries(KinSealDeviceScanTests PRIVATE Qt6::Core Qt6::Test)
target_compile_definitions(KinSealDeviceScanTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

#20261109-0 add: Bound native source primitives without treating an opaque archive as semantic import.
qt_add_executable(KinSealNativeSourceTests src/native_project_codec.cpp tests/native_project_codec_tests.cpp
    src/project_model.cpp src/native_test_project.cpp src/plc_address.cpp src/static_text_properties.cpp src/bit_button_properties.cpp plc_catalog.qrc native_test_template.qrc)
target_include_directories(KinSealNativeSourceTests PRIVATE src)
target_link_libraries(KinSealNativeSourceTests PRIVATE Qt6::Widgets Qt6::Test)
target_compile_definitions(KinSealNativeSourceTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

target_compile_definitions(KinSealStudioLite PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
)

#20261109-0 add: Share the recovered pure address catalog with every project/model consumer.
foreach(KINSEAL_CODEC_TARGET KinSealStudioLite KinSealProjectModelTests KinSealStage1BackendTests KinSealStage2DialogsTests KinSealStage2GlobalToolsTests KinSealStage2WorkflowTests KinSealStage3CanvasTests KinSealStage3ProjectEditsTests KinSealStage3IntegrationTests KinSealStaticTextTests KinSealWordButtonDialogTests KinSealLampDialogTests KinSealFunctionButtonDialogTests KinSealDeviceMacroTests KinSealPlcAddressTests KinSealNativeSourceTests) #20261110-8 change
    target_sources(${KINSEAL_CODEC_TARGET} PRIVATE src/plc_native_catalog.cpp src/plc_native_catalog.h)
endforeach()

#20261110-8 add: Independent property and renderer targets share bounded source-image decoding and the page-pixel font contract.
foreach(KINSEAL_RENDER_TARGET KinSealNativeFunctionControlTests KinSealNativeLampControlTests KinSealBitButtonRendererTests KinSealNativeControlTests KinSealNativeWordControlTests KinSealNativeStaticControlTests KinSealDeviceMacroTests KinSealNativeSourceTests)
    target_sources(${KINSEAL_RENDER_TARGET} PRIVATE src/control_rendering.cpp src/control_rendering.h)
endforeach()

#20261109-6 add: Every independent project-model target must resolve word-property serialization.
foreach(KINSEAL_WORD_MODEL_TARGET KinSealDeviceMacroTests KinSealNativeSourceTests)
    #20261112-0 add: Standalone model consumers must link every typed model now embedded in HmiObject.
    target_sources(${KINSEAL_WORD_MODEL_TARGET} PRIVATE
        src/gif_display_properties.cpp src/number_display_properties.cpp src/ascii_display_properties.cpp
        src/time_display_properties.cpp src/screen_button_properties.cpp src/multi_state_button_properties.cpp
        src/multi_function_button_properties.cpp src/dropdown_properties.cpp src/flow_block_properties.cpp src/slider_properties.cpp src/wheel_properties.cpp src/move_track_properties.cpp src/chart_properties.cpp src/trend_properties.cpp src/history_trend_properties.cpp src/data_collection_properties.cpp src/xy_curve_properties.cpp src/data_group_properties.cpp src/alarm_bar_properties.cpp src/alarm_registration_properties.cpp)
    target_sources(${KINSEAL_WORD_MODEL_TARGET} PRIVATE src/message_display_properties.cpp src/message_display_properties.h) #20261112-0 add
    target_sources(${KINSEAL_WORD_MODEL_TARGET} PRIVATE src/scale_properties.cpp src/scale_properties.h) #20261112-0 add
    target_sources(${KINSEAL_WORD_MODEL_TARGET} PRIVATE src/qr_code_properties.cpp src/qr_code_properties.h) #20261112-0 add
    target_sources(${KINSEAL_WORD_MODEL_TARGET} PRIVATE src/image_display_properties.cpp src/image_display_properties.h) #20261112-0 add
    target_sources(${KINSEAL_WORD_MODEL_TARGET} PRIVATE src/word_button_properties.cpp src/word_button_properties.h)
    target_sources(${KINSEAL_WORD_MODEL_TARGET} PRIVATE src/lamp_properties.cpp src/lamp_properties.h) #20261109-8 add
    target_sources(${KINSEAL_WORD_MODEL_TARGET} PRIVATE src/function_button_properties.cpp src/function_button_properties.h) #20261110-0 add
    target_sources(${KINSEAL_WORD_MODEL_TARGET} PRIVATE src/stage2_system_settings.cpp src/stage2_system_settings.h) #20261110-7 add: Link the strict schema validator in every project-model consumer.
endforeach()

if(MINGW)
    target_compile_options(KinSealStudioLite PRIVATE -Wall -Wextra -Wpedantic)
    target_link_options(KinSealStudioLite PRIVATE -static-libgcc -static-libstdc++)
endif()
