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
#include "quickacrylicmaterial.h"
#include <QtCore/qobject.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE
class QQuickImage;
class QQuickRectangle;
QT_END_NAMESPACE

class QuickGaussianBlur;
class QuickBlend;

class QTACRYLICMATERIAL_API QuickAcrylicMaterialPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QuickAcrylicMaterial)
    Q_DISABLE_COPY_MOVE(QuickAcrylicMaterialPrivate)

public:
    using Theme = QuickAcrylicMaterial::Theme;

    explicit QuickAcrylicMaterialPrivate(QuickAcrylicMaterial *q);
    ~QuickAcrylicMaterialPrivate() override;

    [[nodiscard]] static QuickAcrylicMaterialPrivate *get(QuickAcrylicMaterial *pub);
    [[nodiscard]] static const QuickAcrylicMaterialPrivate *get(const QuickAcrylicMaterial *pub);

    void subscribeSystemThemeChangeNotification();

public Q_SLOTS:
    void updateAcrylicAppearance();
    void rebindWindow();

protected:
    [[nodiscard]] bool eventFilter(QObject *object, QEvent *event) override;

private:
    void createBlurredSource();
    void createLuminosityColorEffect();
    void createLuminosityBlendEffect();
    void createTintColorEffect();
    void createTintBlendEffect();
    void createNoiseBorderEffect();
    void createFallbackColorEffect();
    void initialize();
    [[nodiscard]] qreal calculateTintOpacityModifier(const QColor &tintColor) const;
    [[nodiscard]] QColor calculateLuminosityColor(const QColor &tintColor, const std::optional<qreal> luminosityOpacity) const;
    [[nodiscard]] QColor calculateEffectiveTintColor() const;
    [[nodiscard]] QColor calculateEffectiveLuminosityColor() const;
    [[nodiscard]] bool shouldAppsUseDarkMode() const;

private:
    QuickAcrylicMaterial *q_ptr = nullptr;
    QQuickItem *m_source = nullptr;
    Theme m_theme = Theme::Unknown;
    QColor m_tintColor = {};
    qreal m_tintOpacity = 0.0;
    std::optional<qreal> m_luminosityOpacity = std::nullopt;
    qreal m_noiseOpacity = 0.0;
    QColor m_fallbackColor = {};
    QScopedPointer<QuickGaussianBlur> m_blurredSource;
    QScopedPointer<QQuickRectangle> m_luminosityColorEffect;
    QScopedPointer<QuickBlend> m_luminosityBlendEffect;
    QScopedPointer<QQuickRectangle> m_tintColorEffect;
    QScopedPointer<QuickBlend> m_tintBlendEffect;
    QScopedPointer<QQuickImage> m_noiseBorderEffect;
    QScopedPointer<QQuickRectangle> m_fallbackColorEffect;
    QMetaObject::Connection m_windowActiveChangeConnection = {};
    bool m_useSystemTheme = false;
    bool m_settingSystemTheme = false;
};
