find_package(Qt6 REQUIRED COMPONENTS
    Quick
)

qt_add_executable(Demo
    main.cpp
)

if(WIN32)
    enable_language(RC)
    target_sources(Demo PRIVATE
        demo.rc demo.manifest
    )
endif()

qt_add_qml_module(Demo
    URI Demo
    VERSION 1.0
    DEPENDENCIES
        org.wangwenx190.QtAcrylicMaterial
    QML_FILES
        MainWindow.qml
)

set_target_properties(Demo PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.wangwenx190.qtacrylicmaterial.demo
    MACOSX_BUNDLE_BUNDLE_VERSION 1.0.0.0
    MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_compile_definitions(Demo PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
    QT_NO_KEYWORDS
    QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    QT_NO_FOREACH
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

target_link_libraries(Demo PRIVATE
    Qt::Quick
)

if(MSVC)
    target_compile_options(Demo PRIVATE
        /utf-8 /W4 /WX
    )
else()
    target_compile_options(Demo PRIVATE
        -Wall -Wextra -Werror
    )
endif()