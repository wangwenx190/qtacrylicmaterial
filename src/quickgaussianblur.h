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

#include <QtQml/qqmlregistration.h>
#include <QtQuick/qquickitem.h>

class QuickGaussianBlurPrivate;

class QuickGaussianBlur : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GaussianBlur)
    Q_DECLARE_PRIVATE(QuickGaussianBlur)
    Q_DISABLE_COPY_MOVE(QuickGaussianBlur)

    Q_PROPERTY(QQuickItem* source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged FINAL)
    Q_PROPERTY(int samples READ samples WRITE setSamples NOTIFY samplesChanged FINAL)
    Q_PROPERTY(qreal deviation READ deviation WRITE setDeviation NOTIFY deviationChanged FINAL)
    Q_PROPERTY(bool cached READ isCached WRITE setCached NOTIFY cachedChanged FINAL)

public:
    explicit QuickGaussianBlur(QQuickItem *parent = nullptr);
    ~QuickGaussianBlur() override;

    [[nodiscard]] QQuickItem *source() const;
    void setSource(QQuickItem *item);

    [[nodiscard]] qreal radius() const;
    void setRadius(const qreal value);

    [[nodiscard]] int samples() const;
    void setSamples(const int value);

    [[nodiscard]] qreal deviation() const;
    void setDeviation(const qreal value);

    [[nodiscard]] bool isCached() const;
    void setCached(const bool value);

Q_SIGNALS:
    void sourceChanged();
    void radiusChanged();
    void samplesChanged();
    void deviationChanged();
    void cachedChanged();

private:
    QScopedPointer<QuickGaussianBlurPrivate> d_ptr;
};

QML_DECLARE_TYPE(QuickGaussianBlur)
