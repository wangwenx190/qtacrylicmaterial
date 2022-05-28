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

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GTK3 REQUIRED gtk+-3.0)
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Gui ShaderTools Quick
)

if(QTACRYLICMATERIAL_BUILD_STATIC)
    qt_add_library(${PROJECT_NAME} STATIC)
else()
    qt_add_library(${PROJECT_NAME} SHARED)
endif()
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
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

if(WIN32)
    if(NOT QTACRYLICMATERIAL_BUILD_STATIC)
        enable_language(RC)
        target_sources(${PROJECT_NAME} PRIVATE qtacrylicmaterial.rc)
    endif()
    target_sources(${PROJECT_NAME} PRIVATE
        quickdesktopwallpaper_win.cpp
        quickacrylicmaterial_win.cpp
    )
elseif(APPLE)
    set(_rcc_options -threshold 1)
    if(QTACRYLICMATERIAL_ZSTD_AVAILABLE)
        set(_rcc_options ${_rcc_options} -compress-algo zstd -compress 19)
    else()
        set(_rcc_options ${_rcc_options} -compress-algo zlib -compress 9)
    endif()
    set(_mac_resource)
    qt_add_big_resources(_mac_resource qtacrylicmaterial_mac.qrc OPTIONS ${_rcc_options})
    target_sources(${PROJECT_NAME} PRIVATE
        ${_mac_resource}
        quickdesktopwallpaper_mac.mm
        quickacrylicmaterial_mac.mm
    )
elseif(UNIX)
    target_sources(${PROJECT_NAME} PRIVATE
        quickdesktopwallpaper_linux.cpp
        quickacrylicmaterial_linux.cpp
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
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
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        QTACRYLICMATERIAL_BUILD_STATIC
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::GuiPrivate
    Qt::ShaderToolsPrivate
    Qt::QuickPrivate
)

if(UNIX AND NOT APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        GDK_VERSION_MIN_REQUIRED=GDK_VERSION_3_6
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${GTK3_LIBRARIES}
    )
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${GTK3_INCLUDE_DIRS}
    )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /utf-8 /W3 /WX
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Werror
    )
endif()
