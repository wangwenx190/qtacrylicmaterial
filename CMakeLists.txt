cmake_minimum_required(VERSION 3.20)

project(QtAcrylic VERSION 1.0.0.0 LANGUAGES CXX)

set(PROJECT_VERSION_SHORT ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
set(PROJECT_VENDOR wangwenx190)
set(PROJECT_URI org.${PROJECT_VENDOR}.${PROJECT_NAME})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.3 COMPONENTS ShaderTools Qml Quick REQUIRED)

qt_add_executable(${PROJECT_NAME}
    qtacrylic.qrc
    qgfxsourceproxy_p.h qgfxsourceproxy.cpp
    qgfxshaderbuilder_p.h qgfxshaderbuilder.cpp
    quickgaussianblur.h quickgaussianblur_p.h quickgaussianblur.cpp
    quickacrylicitem.h quickacrylicitem_p.h quickacrylicitem.cpp
    main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI ${PROJECT_URI}
    VERSION ${PROJECT_VERSION_SHORT}
    QML_FILES main.qml
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER ${PROJECT_URI}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_SHORT}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
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

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::ShaderToolsPrivate
    Qt6::QmlPrivate
    Qt6::QuickPrivate
)
