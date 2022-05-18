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

#include "qtacrylicmaterialplugin.h"
#include "qgfxshaderbuilder_p.h"
#include "qgfxsourceproxy_p.h"
#include "quickdesktopwallpaper.h"
#include "quickgaussianblur.h"
#include "quickblend.h"
#include "quickacrylicmaterial.h"
#include <QtQml/qqmlengine.h>

void QtAcrylicMaterial::registerTypes(QQmlEngine *engine)
{
    Q_UNUSED(engine);
    qmlRegisterType<QGfxShaderBuilder>(QTACRYLICMATERIAL_QUICK_URI, 1, 0, "ShaderBuilder");
    qmlRegisterType<QGfxSourceProxy>(QTACRYLICMATERIAL_QUICK_URI, 1, 0, "SourceProxy");
    qmlRegisterType<QuickDesktopWallpaper>(QTACRYLICMATERIAL_QUICK_URI, 1, 0, "DesktopWallpaper");
    qmlRegisterType<QuickGaussianBlur>(QTACRYLICMATERIAL_QUICK_URI, 1, 0, "GaussianBlur");
    qmlRegisterType<QuickBlend>(QTACRYLICMATERIAL_QUICK_URI, 1, 0, "Blend");
    qmlRegisterType<QuickAcrylicMaterial>(QTACRYLICMATERIAL_QUICK_URI, 1, 0, "AcrylicMaterial");
    qmlRegisterModule(QTACRYLICMATERIAL_QUICK_URI, 1, 0);
}
