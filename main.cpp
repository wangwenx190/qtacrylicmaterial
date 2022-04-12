#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlapplicationengine.h>
#include <QtQuick/qquickwindow.h>
#include "qgfxsourceproxy_p.h"
#include "qgfxshaderbuilder_p.h"
#include "quickgaussianblur.h"
#include "quickblend.h"
#include "quickacrylicitem.h"

static constexpr const char QtAcrylicUri[] = "org.wangwenx190.QtAcrylic";

#define QTACRYLIC_FULL_URI QtAcrylicUri, 1, 0

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);

    QGuiApplication application(argc, argv);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);

    QQmlApplicationEngine engine;

    qmlRegisterModule(QTACRYLIC_FULL_URI);
    qmlRegisterType<QGfxSourceProxy>(QTACRYLIC_FULL_URI, "SourceProxy");
    qmlRegisterType<QGfxShaderBuilder>(QTACRYLIC_FULL_URI, "ShaderBuilder");
    qmlRegisterType<QuickGaussianBlur>(QTACRYLIC_FULL_URI, "GaussianBlur");
    qmlRegisterType<QuickBlend>(QTACRYLIC_FULL_URI, "Blend");
    qmlRegisterType<QuickAcrylicItem>(QTACRYLIC_FULL_URI, "AcrylicItem");

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
