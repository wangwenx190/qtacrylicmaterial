set(PROJECT_TARGET qtacrylicmaterialplugin)
set(PROJECT_CLASS QtAcrylicMaterialPlugin)
set(PROJECT_VERSION_SHORT ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
set(PROJECT_VENDOR wangwenx190)
set(PROJECT_URI org.${PROJECT_VENDOR}.${PROJECT_NAME})
set(PROJECT_IMPORT_DIR ${PROJECT_BINARY_DIR}/imports/org/${PROJECT_VENDOR}/${PROJECT_NAME})

find_package(Qt6 REQUIRED COMPONENTS
    Gui ShaderTools Quick
)

qt_add_qml_module(${PROJECT_TARGET}
    URI ${PROJECT_URI}
    VERSION ${PROJECT_VERSION_SHORT}
    PLUGIN_TARGET ${PROJECT_TARGET}
    CLASS_NAME ${PROJECT_CLASS}
    OUTPUT_DIRECTORY ${PROJECT_IMPORT_DIR}
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_PLUGIN_SOURCE
    RESOURCES
        assets/noise_256x256.png
    SOURCES
        plugin.cpp
        qgfxsourceproxy_p.h qgfxsourceproxy.cpp
        qgfxshaderbuilder_p.h qgfxshaderbuilder.cpp
        quickblend.h quickblend_p.h quickblend.cpp
        quickgaussianblur.h quickgaussianblur_p.h quickgaussianblur.cpp
        quickdesktopwallpaper.h quickdesktopwallpaper_p.h quickdesktopwallpaper.cpp
        quickacrylicmaterial.h quickacrylicmaterial_p.h quickacrylicmaterial.cpp
)

if(WIN32)
    target_sources(${PROJECT_TARGET} PRIVATE
        quickdesktopwallpaper_win.cpp
    )
endif()

target_compile_definitions(${PROJECT_TARGET} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
    #QT_NO_KEYWORDS
    QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    QT_NO_FOREACH
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

target_link_libraries(${PROJECT_TARGET} PRIVATE
    Qt::GuiPrivate
    Qt::ShaderToolsPrivate
    Qt::QuickPrivate
)

if(MSVC)
    target_compile_options(${PROJECT_TARGET} PRIVATE
        /utf-8 /W3 /WX
    )
else()
    target_compile_options(${PROJECT_TARGET} PRIVATE
        -Wall -Wextra -Werror
    )
endif()
