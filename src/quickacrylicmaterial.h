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
#include <QtQml/qqmlregistration.h>
#include <QtQuick/qquickitem.h>

class QuickAcrylicMaterialPrivate;

class QTACRYLICMATERIAL_API QuickAcrylicMaterial : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(AcrylicMaterial)
    Q_DECLARE_PRIVATE(QuickAcrylicMaterial)
    Q_DISABLE_COPY_MOVE(QuickAcrylicMaterial)

    Q_PROPERTY(QQuickItem* source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(Theme theme READ theme WRITE setTheme NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor tintColor READ tintColor WRITE setTintColor NOTIFY tintColorChanged FINAL)
    Q_PROPERTY(qreal tintOpacity READ tintOpacity WRITE setTintOpacity NOTIFY tintOpacityChanged FINAL)
    Q_PROPERTY(qreal luminosityOpacity READ luminosityOpacity WRITE setLuminosityOpacity NOTIFY luminosityOpacityChanged FINAL)
    Q_PROPERTY(qreal noiseOpacity READ noiseOpacity WRITE setNoiseOpacity NOTIFY noiseOpacityChanged FINAL)
    Q_PROPERTY(QColor fallbackColor READ fallbackColor WRITE setFallbackColor NOTIFY fallbackColorChanged FINAL)

public:
    enum class Theme
    {
        Unknown = -1, Dark, Light, HighContrast, System, Default = Dark
    };
    Q_ENUM(Theme)

    explicit QuickAcrylicMaterial(QQuickItem *parent = nullptr);
    ~QuickAcrylicMaterial() override;

    [[nodiscard]] QQuickItem *source() const;
    void setSource(QQuickItem *item);

    [[nodiscard]] Theme theme() const;
    void setTheme(const Theme value);

    [[nodiscard]] QColor tintColor() const;
    void setTintColor(const QColor &color);

    [[nodiscard]] qreal tintOpacity() const;
    void setTintOpacity(const qreal opacity);

    [[nodiscard]] qreal luminosityOpacity() const;
    void setLuminosityOpacity(const qreal opacity);

    [[nodiscard]] qreal noiseOpacity() const;
    void setNoiseOpacity(const qreal opacity);

    [[nodiscard]] QColor fallbackColor() const;
    void setFallbackColor(const QColor &color);

protected:
    void itemChange(const ItemChange change, const ItemChangeData &value) override;

Q_SIGNALS:
    void sourceChanged();
    void themeChanged();
    void tintColorChanged();
    void tintOpacityChanged();
    void luminosityOpacityChanged();
    void noiseOpacityChanged();
    void fallbackColorChanged();

private:
    QScopedPointer<QuickAcrylicMaterialPrivate> d_ptr;
};

QML_DECLARE_TYPE(QuickAcrylicMaterial)
