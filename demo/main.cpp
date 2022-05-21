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
#include <qtacrylicmaterialplugin.h>

int main(int argc, char *argv[])
{
    qputenv("QSG_INFO", "1"_qba);

    QCoreApplication::setApplicationName(u"QtAcrylicMaterial Demo"_qs);
    QGuiApplication::setApplicationDisplayName(u"QtAcrylicMaterial Demo"_qs);
    QCoreApplication::setApplicationVersion(u"1.0.0.0"_qs);
    QCoreApplication::setOrganizationName(u"wangwenx190"_qs);
    QCoreApplication::setOrganizationDomain(u"wangwenx190.github.io"_qs);

    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);

    QGuiApplication application(argc, argv);

    QQmlApplicationEngine engine;

    QtAcrylicMaterial::registerTypes(&engine);

    const QUrl mainWindowUrl(u"qrc:///Demo/MainWindow.qml"_qs);

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
