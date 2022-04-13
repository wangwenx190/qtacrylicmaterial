/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlapplicationengine.h>
#include <QtQuick/qquickwindow.h>
#include "qgfxsourceproxy_p.h"
#include "qgfxshaderbuilder_p.h"
#include "quickgaussianblur.h"
#include "quickblend.h"
#include "quickacrylicmaterial.h"

static constexpr const char QtAcrylicMaterialUri[] = "org.wangwenx190.QtAcrylicMaterial";

#define QTACRYLICMATERIAL_FULL_URI QtAcrylicMaterialUri, 1, 0

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);

    QGuiApplication application(argc, argv);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);

    QQmlApplicationEngine engine;

    qmlRegisterModule(QTACRYLICMATERIAL_FULL_URI);
    qmlRegisterType<QGfxSourceProxy>(QTACRYLICMATERIAL_FULL_URI, "SourceProxy");
    qmlRegisterType<QGfxShaderBuilder>(QTACRYLICMATERIAL_FULL_URI, "ShaderBuilder");
    qmlRegisterType<QuickGaussianBlur>(QTACRYLICMATERIAL_FULL_URI, "GaussianBlur");
    qmlRegisterType<QuickBlend>(QTACRYLICMATERIAL_FULL_URI, "Blend");
    qmlRegisterType<QuickAcrylicMaterial>(QTACRYLICMATERIAL_FULL_URI, "AcrylicMaterial");

    const QUrl mainWindowUrl(u"qrc:///org/wangwenx190/QtAcrylic/main.qml"_qs);

    const QMetaObject::Connection connection = QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &application,
        [&mainWindowUrl, &connection](QObject *obj, const QUrl &objUrl) {
            if (objUrl != mainWindowUrl) {
                return;
            }
            if (obj) {
                QObject::disconnect(connection);
            } else {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(mainWindowUrl);

    return QCoreApplication::exec();
}
