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

set(import_dir_path ${PROJECT_BINARY_DIR}/imports)

find_package(Qt6 REQUIRED COMPONENTS Gui Quick)

qt_add_executable(Demo main.cpp)

if(WIN32)
    enable_language(RC)
    target_sources(Demo PRIVATE demo.rc demo.manifest)
endif()

set(QML_SOURCES MainWindow.qml)

qt_add_qml_module(Demo
    URI Demo
    VERSION 1.0
    IMPORTS QtQuick
    DEPENDENCIES org.wangwenx190.QtAcrylicMaterial
    #IMPORT_PATH ${import_dir_path}
    QML_FILES ${QML_SOURCES}
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
    DEMO_IMPORT_DIR="${import_dir_path}"
)

target_link_libraries(Demo PRIVATE
    Qt::Gui Qt::Quick
    QtAcrylicMaterial
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
