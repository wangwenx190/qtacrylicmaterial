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
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE
class QScreen;
class QQuickItem;
class QQuickShaderEffect;
class QQuickShaderEffectSource;
QT_END_NAMESPACE

class QuickGaussianBlur;
class QGfxShaderBuilder;
class QGfxSourceProxy;

class QTACRYLICMATERIAL_API QuickGaussianBlurPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QuickGaussianBlur)
    Q_DISABLE_COPY_MOVE(QuickGaussianBlurPrivate)

public:
    explicit QuickGaussianBlurPrivate(QuickGaussianBlur *q);
    ~QuickGaussianBlurPrivate() override;

    [[nodiscard]] static QuickGaussianBlurPrivate *get(QuickGaussianBlur *pub);
    [[nodiscard]] static const QuickGaussianBlurPrivate *get(const QuickGaussianBlur *pub);

public Q_SLOTS:
    void rebuildShaders();

private Q_SLOTS:
    void updateDpr(const qreal newDpr);

private:
    void initialize();

private:
    QuickGaussianBlur *q_ptr = nullptr;
    QQuickItem *m_source = nullptr;
    qreal m_radius = 0.0;
    int m_samples = 0;
    qreal m_deviation = 0.0;
    bool m_cached = false;
    qreal m_kernelRadius = 0.0;
    int m_kernelSize = 0;
    bool m_alphaOnly = false;
    qreal m_thickness = 0.0;
    qreal m_dpr = 1.0;
    QQuickItem *m_maskSource = nullptr;
    QScopedPointer<QGfxShaderBuilder> m_shaderBuilder;
    QScopedPointer<QGfxSourceProxy> m_sourceProxy;
    QScopedPointer<QQuickShaderEffect> m_horizontalBlur;
    QScopedPointer<QQuickShaderEffect> m_verticalBlur;
    QScopedPointer<QQuickShaderEffectSource> m_cacheItem;
};
