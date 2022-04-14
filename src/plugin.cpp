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

#include <QtQml/qqmlextensionplugin.h>
#include "qgfxsourceproxy_p.h"
#include "qgfxshaderbuilder_p.h"
#include "quickgaussianblur.h"
#include "quickblend.h"
#include "quickdesktopwallpaper.h"
#include "quickacrylicmaterial.h"

static constexpr const char QtAcrylicMaterialUri[] = "org.wangwenx190.QtAcrylicMaterial";

#ifndef QTACRYLICMATERIAL_FULL_URI
#  define QTACRYLICMATERIAL_FULL_URI QtAcrylicMaterialUri, 1, 0
#endif

class QtAcrylicMaterialPlugin : public QQmlEngineExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid)

public:
    explicit QtAcrylicMaterialPlugin(QObject *parent = nullptr) : QQmlEngineExtensionPlugin(parent) {}
    ~QtAcrylicMaterialPlugin() override = default;

    void initializeEngine(QQmlEngine *engine, const char *uri) override
    {
        Q_ASSERT(engine);
        Q_ASSERT(uri);
        if (!engine || !uri) {
            return;
        }
        Q_ASSERT(qstrcmp(uri, QtAcrylicMaterialUri) == 0);
        if (qstrcmp(uri, QtAcrylicMaterialUri) != 0) {
            return;
        }
        qmlRegisterType<QGfxSourceProxy>(QTACRYLICMATERIAL_FULL_URI, "SourceProxy");
        qmlRegisterType<QGfxShaderBuilder>(QTACRYLICMATERIAL_FULL_URI, "ShaderBuilder");
        qmlRegisterType<QuickGaussianBlur>(QTACRYLICMATERIAL_FULL_URI, "GaussianBlur");
        qmlRegisterType<QuickBlend>(QTACRYLICMATERIAL_FULL_URI, "Blend");
        qmlRegisterType<QuickDesktopWallpaper>(QTACRYLICMATERIAL_FULL_URI, "DesktopWallpaper");
        qmlRegisterType<QuickAcrylicMaterial>(QTACRYLICMATERIAL_FULL_URI, "AcrylicMaterial");
        qmlRegisterModule(QTACRYLICMATERIAL_FULL_URI);
    }
};

#include "plugin.moc"
