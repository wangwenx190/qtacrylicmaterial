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

#include "quickacrylicmaterial.h"
#include "quickacrylicmaterial_p.h"
#include "quickgaussianblur.h"
#include "quickblend.h"
#include <QtGui/qpa/qplatformtheme.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <QtQuick/private/qquickimage_p.h>
#include <QtQuick/private/qquickrectangle_p.h>

static constexpr const QColor sc_defaultTintColor = { 255, 255, 255, 204 };
static constexpr const qreal sc_defaultTintOpacity = 1.0;
static constexpr const qreal sc_defaultNoiseOpacity = 0.02;
static constexpr const QColor sc_defaultFallbackColor = QColorConstants::Black;
[[maybe_unused]] static constexpr const qreal sc_defaultBlurRadius = 30.0;
[[maybe_unused]] static constexpr const QColor sc_defaultExclusionColor = { 255, 255, 255, 26 };
[[maybe_unused]] static constexpr const qreal sc_defaultSaturation = 1.25;

namespace Preset
{
namespace Dark
{
static constexpr const QColor sc_defaultTintColor = {44, 44, 44}; // #2C2C2C
static constexpr const qreal sc_defaultTintOpacity = 0.15;
static constexpr const qreal sc_defaultLuminosityOpacity = 0.96;
static constexpr const QColor sc_defaultFallbackColor = {44, 44, 44}; // #2C2C2C
} // namespace Dark
namespace Light
{
static constexpr const QColor sc_defaultTintColor = {252, 252, 252}; // #FCFCFC
static constexpr const qreal sc_defaultTintOpacity = 0.0;
static constexpr const qreal sc_defaultLuminosityOpacity = 0.85;
static constexpr const QColor sc_defaultFallbackColor = {249, 249, 249}; // #F9F9F9
} // namespace Light
namespace HighContrast
{
// ### TODO
} // namespace HighContrast
} // namespace Preset

static inline void initResource()
{
    Q_INIT_RESOURCE(qtacrylicmaterial);
}

QuickAcrylicMaterialPrivate::QuickAcrylicMaterialPrivate(QuickAcrylicMaterial *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

QuickAcrylicMaterialPrivate::~QuickAcrylicMaterialPrivate() = default;

QuickAcrylicMaterialPrivate *QuickAcrylicMaterialPrivate::get(QuickAcrylicMaterial *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const QuickAcrylicMaterialPrivate *QuickAcrylicMaterialPrivate::get(const QuickAcrylicMaterial *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

void QuickAcrylicMaterialPrivate::updateAcrylicAppearance()
{
    Q_Q(QuickAcrylicMaterial);

    m_luminosityColorEffect->setColor(calculateEffectiveLuminosityColor(m_tintColor, m_tintOpacity, m_luminosityOpacity));
    m_tintColorEffect->setColor(calculateEffectiveTintColor(m_tintColor, m_tintOpacity, m_luminosityOpacity));
    m_noiseBorderEffect->setOpacity(m_noiseOpacity);
    m_fallbackColorEffect->setColor(m_fallbackColor);

    const bool active = (q->window() ? q->window()->isActive() : false);
    m_tintBlendEffect->setVisible(active);
    m_noiseBorderEffect->setVisible(active);
    m_fallbackColorEffect->setVisible(!active);
}

void QuickAcrylicMaterialPrivate::rebindWindow()
{
    Q_Q(QuickAcrylicMaterial);
    QQuickWindow * const window = q->window();
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    if (m_windowActiveChangeConnection) {
        disconnect(m_windowActiveChangeConnection);
        m_windowActiveChangeConnection = {};
    }
    m_windowActiveChangeConnection = connect(window, &QQuickWindow::activeChanged,
                       this, &QuickAcrylicMaterialPrivate::updateAcrylicAppearance);
}

bool QuickAcrylicMaterialPrivate::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (!object || !event) {
        return false;
    }
    if (!m_useSystemTheme) {
        return QObject::eventFilter(object, event);
    }
    switch (event->type()) {
    case QEvent::ThemeChange: {
        qDebug() << "Detected theme change event.";
        Q_Q(QuickAcrylicMaterial);
        q->setTheme(Theme::System);
    } break;
    case QEvent::ApplicationPaletteChange: {
        qDebug() << "Detected application palette change event.";
        Q_Q(QuickAcrylicMaterial);
        q->setTheme(Theme::System);
    } break;
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

void QuickAcrylicMaterialPrivate::createBlurredSource()
{
    Q_Q(QuickAcrylicMaterial);
    m_blurredSource.reset(new QuickGaussianBlur(q));
    static constexpr const qreal maximumBlurRadius = 60.0;
    m_blurredSource->setRadius(maximumBlurRadius);
    // https://doc.qt.io/qt-6/qml-qtgraphicaleffects-gaussianblur.html#samples-prop
    // Ideally, the blur samples should be twice as large as the highest required radius value plus one.
    static constexpr const auto maximumBlurSamples = int(qRound(maximumBlurRadius * 2.0));
    m_blurredSource->setSamples(maximumBlurSamples);
    m_blurredSource->setVisible(false);
    const auto blurredSourceAnchors = new QQuickAnchors(m_blurredSource.get(), m_blurredSource.get());
    blurredSourceAnchors->setFill(q);
}

void QuickAcrylicMaterialPrivate::createLuminosityColorEffect()
{
    Q_Q(QuickAcrylicMaterial);
    m_luminosityColorEffect.reset(new QQuickRectangle(q));
    QQuickPen * const border = m_luminosityColorEffect->border();
    border->setWidth(0.0);
    border->setColor(QColorConstants::Transparent);
    m_luminosityColorEffect->setVisible(false);
    const auto luminosityColorEffectAnchors = new QQuickAnchors(m_luminosityColorEffect.get(), m_luminosityColorEffect.get());
    luminosityColorEffectAnchors->setFill(q);
}

void QuickAcrylicMaterialPrivate::createLuminosityBlendEffect()
{
    Q_Q(QuickAcrylicMaterial);
    m_luminosityBlendEffect.reset(new QuickBlend(q));
    m_luminosityBlendEffect->setMode(QuickBlend::Mode::Lightness);
    m_luminosityBlendEffect->setBackground(m_blurredSource.get());
    m_luminosityBlendEffect->setForeground(m_luminosityColorEffect.get());
    m_luminosityBlendEffect->setVisible(false);
    const auto luminosityBlendEffectAnchors = new QQuickAnchors(m_luminosityBlendEffect.get(), m_luminosityBlendEffect.get());
    luminosityBlendEffectAnchors->setFill(q);
}

void QuickAcrylicMaterialPrivate::createTintColorEffect()
{
    Q_Q(QuickAcrylicMaterial);
    m_tintColorEffect.reset(new QQuickRectangle(q));
    QQuickPen * const border = m_tintColorEffect->border();
    border->setWidth(0.0);
    border->setColor(QColorConstants::Transparent);
    m_tintColorEffect->setVisible(false);
    const auto tintColorEffectAnchors = new QQuickAnchors(m_tintColorEffect.get(), m_tintColorEffect.get());
    tintColorEffectAnchors->setFill(q);
}

void QuickAcrylicMaterialPrivate::createTintBlendEffect()
{
    Q_Q(QuickAcrylicMaterial);
    m_tintBlendEffect.reset(new QuickBlend(q));
    m_tintBlendEffect->setMode(QuickBlend::Mode::Color);
    m_tintBlendEffect->setBackground(m_luminosityBlendEffect.get());
    m_tintBlendEffect->setForeground(m_tintColorEffect.get());
    m_tintBlendEffect->setVisible(false);
    const auto tintBlendEffectAnchors = new QQuickAnchors(m_tintBlendEffect.get(), m_tintBlendEffect.get());
    tintBlendEffectAnchors->setFill(q);
}

void QuickAcrylicMaterialPrivate::createNoiseBorderEffect()
{
    Q_Q(QuickAcrylicMaterial);
    m_noiseBorderEffect.reset(new QQuickImage(q));
    initResource();
    m_noiseBorderEffect->setSource(QUrl(u"qrc:///org/wangwenx190/QtAcrylicMaterial/assets/noise_256x256.png"_qs));
    m_noiseBorderEffect->setFillMode(QQuickImage::Tile);
    m_noiseBorderEffect->setVisible(false);
    const auto noiseBorderEffectAnchors = new QQuickAnchors(m_noiseBorderEffect.get(), m_noiseBorderEffect.get());
    noiseBorderEffectAnchors->setFill(q);
}

void QuickAcrylicMaterialPrivate::createFallbackColorEffect()
{
    Q_Q(QuickAcrylicMaterial);
    m_fallbackColorEffect.reset(new QQuickRectangle(q));
    QQuickPen * const border = m_fallbackColorEffect->border();
    border->setWidth(0.0);
    border->setColor(QColorConstants::Transparent);
    m_fallbackColorEffect->setVisible(false);
    const auto fallbackColorEffectAnchors = new QQuickAnchors(m_fallbackColorEffect.get(), m_fallbackColorEffect.get());
    fallbackColorEffectAnchors->setFill(q);
}

void QuickAcrylicMaterialPrivate::initialize()
{
    Q_Q(QuickAcrylicMaterial);
    q->setClip(true);

    connect(q, &QuickAcrylicMaterial::tintColorChanged, this, &QuickAcrylicMaterialPrivate::updateAcrylicAppearance);
    connect(q, &QuickAcrylicMaterial::tintOpacityChanged, this, &QuickAcrylicMaterialPrivate::updateAcrylicAppearance);
    connect(q, &QuickAcrylicMaterial::luminosityOpacityChanged, this, &QuickAcrylicMaterialPrivate::updateAcrylicAppearance);
    connect(q, &QuickAcrylicMaterial::noiseOpacityChanged, this, &QuickAcrylicMaterialPrivate::updateAcrylicAppearance);
    connect(q, &QuickAcrylicMaterial::fallbackColorChanged, this, &QuickAcrylicMaterialPrivate::updateAcrylicAppearance);

    m_tintColor = sc_defaultTintColor;
    m_tintOpacity = sc_defaultTintOpacity;
    m_noiseOpacity = sc_defaultNoiseOpacity;
    m_fallbackColor = sc_defaultFallbackColor;

    createBlurredSource();
    createLuminosityColorEffect();
    createLuminosityBlendEffect();
    createTintColorEffect();
    createTintBlendEffect();
    createNoiseBorderEffect();
    createFallbackColorEffect();

    updateAcrylicAppearance();

    subscribeSystemThemeChangeNotification();
}

qreal QuickAcrylicMaterialPrivate::calculateTintOpacityModifier(const QColor &tintColor)
{
    // This method supresses the maximum allowable tint opacity depending on the luminosity and saturation of a color by
    // compressing the range of allowable values - for example, a user-defined value of 100% will be mapped to 45% for pure
    // white (100% luminosity), 85% for pure black (0% luminosity), and 90% for pure gray (50% luminosity).  The intensity of
    // the effect increases linearly as luminosity deviates from 50%.  After this effect is calculated, we cancel it out
    // linearly as saturation increases from zero.

    const qreal midPoint = 0.50; // Mid point of HsvV range that these calculations are based on. This is here for easy tuning.

    const qreal whiteMaxOpacity = 0.45; // 100% luminosity
    const qreal midPointMaxOpacity = 0.90; // 50% luminosity
    const qreal blackMaxOpacity = 0.85; // 0% luminosity

    const QColor rgb = tintColor.toRgb();
    const QColor hsv = rgb.toHsv();

    qreal opacityModifier = midPointMaxOpacity;

    if (!qFuzzyCompare(qreal(hsv.valueF()), midPoint)) {
        // Determine maximum suppression amount
        qreal lowestMaxOpacity = midPointMaxOpacity;
        qreal maxDeviation = midPoint;

        if (hsv.valueF() > midPoint) {
            lowestMaxOpacity = whiteMaxOpacity; // At white (100% hsvV)
            maxDeviation = (1.0 - maxDeviation);
        } else if (hsv.valueF() < midPoint) {
            lowestMaxOpacity = blackMaxOpacity; // At black (0% hsvV)
        }

        qreal maxOpacitySuppression = (midPointMaxOpacity - lowestMaxOpacity);

        // Determine normalized deviation from the midpoint
        const qreal deviation = qAbs(hsv.valueF() - midPoint);
        const qreal normalizedDeviation = (deviation / maxDeviation);

        // If we have saturation, reduce opacity suppression to allow that color to come through more
        if (hsv.saturationF() > 0) {
            // Dampen opacity suppression based on how much saturation there is
            maxOpacitySuppression *= qMax((1.0 - (hsv.saturationF() * 2.0)), 0.0);
        }

        const qreal opacitySuppression = (maxOpacitySuppression * normalizedDeviation);

        opacityModifier = (midPointMaxOpacity - opacitySuppression);
    }

    return opacityModifier;
}

QColor QuickAcrylicMaterialPrivate::calculateEffectiveTintColor(const QColor &tintColor, const qreal tintOpacity, const std::optional<qreal> luminosityOpacity)
{
    QColor color = tintColor;

    // Update tint color's alpha with the combined opacity value.
    // If luminosity opacity was specified, we don't intervene into users parameters.
    if (luminosityOpacity.has_value()) {
        color.setAlphaF(color.alphaF() * tintOpacity);
    } else {
        const qreal tintOpacityModifier = calculateTintOpacityModifier(color);
        color.setAlphaF(color.alphaF() * tintOpacity * tintOpacityModifier);
    }

    return color;
}

QColor QuickAcrylicMaterialPrivate::calculateEffectiveLuminosityColor(const QColor &tintColor, const qreal tintOpacity, const std::optional<qreal> luminosityOpacity)
{
    QColor color = tintColor;

    // Purposely leaving out tint opacity modifier here because calculateLuminosityColor() needs the *original* tint opacity set by the user.
    color.setAlphaF(color.alphaF() * tintOpacity);

    return calculateLuminosityColor(color, luminosityOpacity);
}

QColor QuickAcrylicMaterialPrivate::calculateLuminosityColor(const QColor &tintColor, const std::optional<qreal> luminosityOpacity)
{
    // The tint color passed into this method should be the original, unmodified color created using user values for TintColor + TintOpacity
    const QColor rgbTintColor = tintColor.toRgb();

    // If luminosity opacity is specified, just use the values as is
    if (luminosityOpacity.has_value()) {
        QColor result = rgbTintColor;
        result.setAlphaF(qBound(0.0, luminosityOpacity.value(), 1.0));
        return result;
    } else {
        // To create the Luminosity blend input color without luminosity opacity,
        // we're taking the TintColor input, converting to HSV, and clamping the V between these values
        const qreal minHsvV = 0.125;
        const qreal maxHsvV = 0.965;

        const QColor hsvTintColor = rgbTintColor.toHsv();

        const qreal clampedHsvV = qBound(minHsvV, hsvTintColor.valueF(), maxHsvV);

        const QColor hsvLuminosityColor = QColor::fromHsvF(hsvTintColor.hueF(), hsvTintColor.saturationF(), clampedHsvV);
        const QColor rgbLuminosityColor = hsvLuminosityColor.toRgb();

        // Now figure out luminosity opacity
        // Map original *tint* opacity to this range
        const qreal minLuminosityOpacity = 0.15;
        const qreal maxLuminosityOpacity = 1.03;

        const qreal luminosityOpacityRangeMax = (maxLuminosityOpacity - minLuminosityOpacity);
        const qreal mappedTintOpacity = ((tintColor.alphaF() * luminosityOpacityRangeMax) + minLuminosityOpacity);

        // Finally, combine the luminosity opacity and the HsvV-clamped tint color
        QColor result = rgbLuminosityColor;
        result.setAlphaF(qMin(mappedTintOpacity, 1.0));
        return result;
    }
}

bool QuickAcrylicMaterialPrivate::shouldAppsUseDarkMode()
{
    if (const QPlatformTheme * const theme = QGuiApplicationPrivate::platformTheme()) {
        return (theme->appearance() == QPlatformTheme::Appearance::Dark);
    }
    return false;
}

QuickAcrylicMaterial::QuickAcrylicMaterial(QQuickItem *parent)
    : QQuickItem(parent), d_ptr(new QuickAcrylicMaterialPrivate(this))
{
    qRegisterMetaType<Theme>();
}

QuickAcrylicMaterial::~QuickAcrylicMaterial() = default;

QQuickItem *QuickAcrylicMaterial::source() const
{
    Q_D(const QuickAcrylicMaterial);
    return d->m_source;
}

void QuickAcrylicMaterial::setSource(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_D(QuickAcrylicMaterial);
    if (d->m_source == item) {
        return;
    }
    d->m_source = item;
    d->m_blurredSource->setSource(d->m_source);
    Q_EMIT sourceChanged();
}

QuickAcrylicMaterial::Theme QuickAcrylicMaterial::theme() const
{
    Q_D(const QuickAcrylicMaterial);
    return (d->m_useSystemTheme ? Theme::System : d->m_theme);
}

void QuickAcrylicMaterial::setTheme(const Theme value)
{
    Q_ASSERT(value != Theme::Unknown);
    if (value == Theme::Unknown) {
        return;
    }
    Q_D(QuickAcrylicMaterial);
    if (value == Theme::System) {
        d->m_settingSystemTheme = true;
    } else {
        if (d->m_theme == value) {
            d->m_settingSystemTheme = false;
            return;
        }
        d->m_theme = value;
        Q_EMIT themeChanged();
    }
    switch (value) {
    case Theme::Unknown:
        Q_ASSERT(false);
        break;
    case Theme::Dark: {
        if (!d->m_settingSystemTheme) {
            d->m_useSystemTheme = false;
        }
        d->m_settingSystemTheme = false;
        setTintColor(Preset::Dark::sc_defaultTintColor);
        setTintOpacity(Preset::Dark::sc_defaultTintOpacity);
        setLuminosityOpacity(Preset::Dark::sc_defaultLuminosityOpacity);
        setFallbackColor(Preset::Dark::sc_defaultFallbackColor);
    } break;
    case Theme::Light: {
        if (!d->m_settingSystemTheme) {
            d->m_useSystemTheme = false;
        }
        d->m_settingSystemTheme = false;
        setTintColor(Preset::Light::sc_defaultTintColor);
        setTintOpacity(Preset::Light::sc_defaultTintOpacity);
        setLuminosityOpacity(Preset::Light::sc_defaultLuminosityOpacity);
        setFallbackColor(Preset::Light::sc_defaultFallbackColor);
    } break;
    case Theme::HighContrast: {
        if (!d->m_settingSystemTheme) {
            d->m_useSystemTheme = false;
        }
        d->m_settingSystemTheme = false;
        // ### TODO
    } break;
    case Theme::System: {
        d->m_settingSystemTheme = true;
        d->m_useSystemTheme = true;
        setTheme(d->shouldAppsUseDarkMode() ? Theme::Dark : Theme::Light);
    } break;
    }
}

QColor QuickAcrylicMaterial::tintColor() const
{
    Q_D(const QuickAcrylicMaterial);
    return d->m_tintColor;
}

void QuickAcrylicMaterial::setTintColor(const QColor &color)
{
    Q_ASSERT(color.isValid());
    if (!color.isValid()) {
        return;
    }
    Q_D(QuickAcrylicMaterial);
    if (d->m_tintColor == color) {
        return;
    }
    d->m_tintColor = color;
    Q_EMIT tintColorChanged();
}

qreal QuickAcrylicMaterial::tintOpacity() const
{
    Q_D(const QuickAcrylicMaterial);
    return d->m_tintOpacity;
}

void QuickAcrylicMaterial::setTintOpacity(const qreal opacity)
{
    Q_D(QuickAcrylicMaterial);
    if (qFuzzyCompare(d->m_tintOpacity, opacity)) {
        return;
    }
    d->m_tintOpacity = opacity;
    Q_EMIT tintOpacityChanged();
}

qreal QuickAcrylicMaterial::luminosityOpacity() const
{
    Q_D(const QuickAcrylicMaterial);
    return (d->m_luminosityOpacity.has_value() ? d->m_luminosityOpacity.value() : 0.0);
}

void QuickAcrylicMaterial::setLuminosityOpacity(const qreal opacity)
{
    Q_D(QuickAcrylicMaterial);
    if (d->m_luminosityOpacity.has_value() && qFuzzyCompare(d->m_luminosityOpacity.value(), opacity)) {
        return;
    }
    d->m_luminosityOpacity = opacity;
    Q_EMIT luminosityOpacityChanged();
}

qreal QuickAcrylicMaterial::noiseOpacity() const
{
    Q_D(const QuickAcrylicMaterial);
    return d->m_noiseOpacity;
}

void QuickAcrylicMaterial::setNoiseOpacity(const qreal opacity)
{
    Q_D(QuickAcrylicMaterial);
    if (qFuzzyCompare(d->m_noiseOpacity, opacity)) {
        return;
    }
    d->m_noiseOpacity = opacity;
    Q_EMIT noiseOpacityChanged();
}

QColor QuickAcrylicMaterial::fallbackColor() const
{
    Q_D(const QuickAcrylicMaterial);
    return d->m_fallbackColor;
}

void QuickAcrylicMaterial::setFallbackColor(const QColor &color)
{
    Q_ASSERT(color.isValid());
    if (!color.isValid()) {
        return;
    }
    Q_D(QuickAcrylicMaterial);
    if (d->m_fallbackColor == color) {
        return;
    }
    d->m_fallbackColor = color;
    Q_EMIT fallbackColorChanged();
}

void QuickAcrylicMaterial::itemChange(const ItemChange change, const ItemChangeData &value)
{
    QQuickItem::itemChange(change, value);
    Q_D(QuickAcrylicMaterial);
    if ((change == ItemSceneChange) && value.window) {
        d->rebindWindow();
        d->updateAcrylicAppearance();
#ifndef Q_OS_WINDOWS
        value.window->installEventFilter(d);
#endif
    }
}
