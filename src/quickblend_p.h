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

#pragma once

#include "qtacrylicmaterial_global.h"
#include "quickblend.h"
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE
class QGfxSourceProxy;
class QGfxShaderBuilder;
class QQuickShaderEffectSource;
class QQuickShaderEffect;
QT_END_NAMESPACE

class QTACRYLICMATERIAL_API QuickBlendPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QuickBlend)
    Q_DISABLE_COPY_MOVE(QuickBlendPrivate)

public:
    using Mode = QuickBlend::Mode;

    explicit QuickBlendPrivate(QuickBlend *q);
    ~QuickBlendPrivate() override;

    [[nodiscard]] static QuickBlendPrivate *get(QuickBlend *pub);
    [[nodiscard]] static const QuickBlendPrivate *get(const QuickBlend *pub);

public Q_SLOTS:
    void buildFragmentShader();

private:
    void initialize();
    [[nodiscard]] QByteArray generateShaderCode(const Mode mode) const;

private:
    QuickBlend *q_ptr = nullptr;
    QQuickItem *m_background = nullptr;
    QQuickItem *m_foreground = nullptr;
    Mode m_mode = Mode::Normal;
    bool m_cached = false;
    QScopedPointer<QGfxShaderBuilder> m_shaderBuilder;
    QScopedPointer<QGfxSourceProxy> m_backgroundSourceProxy;
    QScopedPointer<QGfxSourceProxy> m_foregroundSourceProxy;
    QScopedPointer<QQuickShaderEffectSource> m_cacheItem;
    QScopedPointer<QQuickShaderEffect> m_shaderItem;
};
