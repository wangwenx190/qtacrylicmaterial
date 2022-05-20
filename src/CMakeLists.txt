#[[
  MIT License

  Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
]]

set(PROJECT_TARGET QtAcrylicMaterial)
set(PROJECT_CLASS QtAcrylicMaterialPlugin)
set(PROJECT_VERSION_SHORT ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
set(PROJECT_VENDOR wangwenx190)
set(PROJECT_URI org.${PROJECT_VENDOR}.${PROJECT_NAME})
set(PROJECT_IMPORT_DIR ${PROJECT_BINARY_DIR}/imports/org/${PROJECT_VENDOR}/${PROJECT_NAME})

find_package(Qt6 REQUIRED COMPONENTS
    Gui ShaderTools Quick
)

if(QTACRYLICMATERIAL_BUILD_STATIC)
    qt_add_library(${PROJECT_TARGET} STATIC)
else()
    qt_add_library(${PROJECT_TARGET} SHARED)
endif()

target_sources(${PROJECT_TARGET} PRIVATE
    qtacrylicmaterial.qrc
    qtacrylicmaterial_global.h
    qgfxsourceproxy_p.h qgfxsourceproxy.cpp
    qgfxshaderbuilder_p.h qgfxshaderbuilder.cpp
    quickblend.h quickblend_p.h quickblend.cpp
    quickgaussianblur.h quickgaussianblur_p.h quickgaussianblur.cpp
    quickdesktopwallpaper.h quickdesktopwallpaper_p.h quickdesktopwallpaper.cpp
    quickacrylicmaterial.h quickacrylicmaterial_p.h quickacrylicmaterial.cpp
    qtacrylicmaterialplugin.h qtacrylicmaterialplugin.cpp
)

#[[qt_add_qml_module(${PROJECT_TARGET}
    URI ${PROJECT_URI}
    VERSION ${PROJECT_VERSION_SHORT}
    CLASS_NAME ${PROJECT_CLASS}
    OUTPUT_DIRECTORY ${PROJECT_IMPORT_DIR}
)]]

if(WIN32)
    if(NOT QTACRYLICMATERIAL_BUILD_STATIC)
        enable_language(RC)
        target_sources(${PROJECT_TARGET} PRIVATE qtacrylicmaterial.rc)
    endif()
    target_sources(${PROJECT_TARGET} PRIVATE
        quickdesktopwallpaper_win.cpp
        quickacrylicmaterial_win.cpp
    )
elseif(APPLE)
    target_sources(${PROJECT_TARGET} PRIVATE
        quickdesktopwallpaper_mac.mm
        quickacrylicmaterial_mac.mm
    )
elseif(UNIX)
    target_sources(${PROJECT_TARGET} PRIVATE
        quickdesktopwallpaper_linux.cpp
        quickacrylicmaterial_linux.cpp
    )
endif()

target_compile_definitions(${PROJECT_TARGET} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
    #QT_NO_KEYWORDS # Some private Qt headers are not keyword-clean.
    QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    QT_NO_FOREACH
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
    QTACRYLICMATERIAL_BUILD_LIBRARY
)

if(QTACRYLICMATERIAL_BUILD_STATIC)
    target_compile_definitions(${PROJECT_TARGET} PUBLIC
        QTACRYLICMATERIAL_BUILD_STATIC
    )
endif()

target_link_libraries(${PROJECT_TARGET} PRIVATE
    Qt::GuiPrivate
    Qt::ShaderToolsPrivate
    Qt::QuickPrivate
)

target_include_directories(${PROJECT_TARGET} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
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
