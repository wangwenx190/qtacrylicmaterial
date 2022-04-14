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

class QuickBlendPrivate;

class QuickBlend : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Blend)
    Q_DECLARE_PRIVATE(QuickBlend)
    Q_DISABLE_COPY_MOVE(QuickBlend)

    Q_PROPERTY(QQuickItem* background READ background WRITE setBackground NOTIFY backgroundChanged FINAL)
    Q_PROPERTY(QQuickItem* foreground READ foreground WRITE setForeground NOTIFY foregroundChanged FINAL)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged FINAL)
    Q_PROPERTY(bool cached READ isCached WRITE setCached NOTIFY cachedChanged FINAL)

public:
    enum class Mode
    {
        Normal, Addition, Average, Color, ColorBurn, ColorDodge,
        Darken, DarkerColor, Difference, Divide, Exclusion, HardLight,
        Hue, Lighten, LighterColor, Lightness, Multiply, Negation,
        Saturation, Screen, Subtract, SoftLight
    };
    Q_ENUM(Mode)

    explicit QuickBlend(QQuickItem *parent = nullptr);
    ~QuickBlend() override;

    [[nodiscard]] QQuickItem *background() const;
    void setBackground(QQuickItem *item);

    [[nodiscard]] QQuickItem *foreground() const;
    void setForeground(QQuickItem *item);

    [[nodiscard]] Mode mode() const;
    void setMode(const Mode value);

    [[nodiscard]] bool isCached() const;
    void setCached(const bool value);

Q_SIGNALS:
    void backgroundChanged();
    void foregroundChanged();
    void modeChanged();
    void cachedChanged();

private:
    QScopedPointer<QuickBlendPrivate> d_ptr;
};

QML_DECLARE_TYPE(QuickBlend)
