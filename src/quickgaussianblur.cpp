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

#include "quickgaussianblur.h"
#include "quickgaussianblur_p.h"
#include "qgfxsourceproxy_p.h"
#include "qgfxshaderbuilder_p.h"
#include <QtCore/qmath.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qscreen.h>
#include <QtQml/qqmlengine.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/private/qquickshadereffect_p.h>
#include <QtQuick/private/qquickshadereffectsource_p.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <QtQuick/private/qquickitem_p.h>

static constexpr const char kSource[] = "source";
static constexpr const char kSpread[] = "spread";
static constexpr const char kDirstep[] = "dirstep";
static constexpr const char kDeviation[] = "deviation";
static constexpr const char kColor[] = "color";
static constexpr const char kThickness[] = "thickness";
static constexpr const char kMask[] = "mask";

QuickGaussianBlurPrivate::QuickGaussianBlurPrivate(QuickGaussianBlur *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

QuickGaussianBlurPrivate::~QuickGaussianBlurPrivate() = default;

QuickGaussianBlurPrivate *QuickGaussianBlurPrivate::get(QuickGaussianBlur *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const QuickGaussianBlurPrivate *QuickGaussianBlurPrivate::get(const QuickGaussianBlur *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

void QuickGaussianBlurPrivate::rebuildShaders()
{
    Q_Q(QuickGaussianBlur);

    m_samples = ((m_samples <= 0) ? 9 : m_samples);
    m_radius = ((m_radius <= 0.0) ? qFloor(qreal(m_samples) / 2.0) : m_radius);

    m_deviation = ((m_radius + 1.0) / 3.3333);
    m_kernelRadius = qMax(0.0, (qreal(m_samples) / 2.0));
    m_kernelSize = int(qRound((m_kernelRadius * 2.0) + 1.0));

    const QVariant spreadVar = (m_radius / m_kernelRadius);
    const QVariant deviationVar = m_deviation;
    const QVariant thicknessVar = qMax(0.0, qMin(0.98, (1.0 - (m_thickness * 0.98))));
    const QVariant maskVar = QVariant::fromValue(m_maskSource);

    m_horizontalBlur->setProperty(kSource, QVariant::fromValue(m_sourceProxy->output()));
    m_horizontalBlur->setProperty(kSpread, spreadVar);
    m_horizontalBlur->setProperty(kDirstep, QVector2D((1.0 / (q->width() * m_dpr)), 0.0));
    m_horizontalBlur->setProperty(kDeviation, deviationVar);
    m_horizontalBlur->setProperty(kColor, QColorConstants::White);
    m_horizontalBlur->setProperty(kThickness, thicknessVar);
    m_horizontalBlur->setProperty(kMask, maskVar);

    m_verticalBlur->setProperty(kSource, QVariant::fromValue(m_horizontalBlur.get()));
    m_verticalBlur->setProperty(kSpread, spreadVar);
    m_verticalBlur->setProperty(kDirstep, QVector2D(0.0, (1.0 / (q->height() * m_dpr))));
    m_verticalBlur->setProperty(kDeviation, deviationVar);
    m_verticalBlur->setProperty(kColor, QColorConstants::Black);
    m_verticalBlur->setProperty(kThickness, thicknessVar);
    m_verticalBlur->setProperty(kMask, maskVar);

    QQmlEngine engine(this);
    QJSValue params = engine.newObject();
    params.setProperty(u"radius"_qs, m_kernelRadius);
    // Limit deviation to something very small avoid getting NaN in the shader.
    params.setProperty(u"deviation"_qs, qMax(0.00001, m_deviation));
    params.setProperty(u"alphaOnly"_qs, m_alphaOnly);
    params.setProperty(u"masked"_qs, (m_maskSource != nullptr));
    params.setProperty(u"fallback"_qs, !qFuzzyCompare(m_radius, m_kernelRadius));
    const QVariantMap shaders = m_shaderBuilder->gaussianBlur(params);
    const QUrl fragmentShaderUrl = shaders.value(u"fragmentShader"_qs).toUrl();
    const QUrl vertexShaderUrl = shaders.value(u"vertexShader"_qs).toUrl();
    m_horizontalBlur->setFragmentShader(fragmentShaderUrl);
    m_horizontalBlur->setVertexShader(vertexShaderUrl);
    m_verticalBlur->setFragmentShader(fragmentShaderUrl);
    m_verticalBlur->setVertexShader(vertexShaderUrl);
}

void QuickGaussianBlurPrivate::updateDpr(const qreal newDpr)
{
    if (qFuzzyCompare(m_dpr, newDpr)) {
        return;
    }
    m_dpr = newDpr;
    rebuildShaders();
}

void QuickGaussianBlurPrivate::initialize()
{
    Q_Q(QuickGaussianBlur);
    connect(q, &QuickGaussianBlur::widthChanged, this, &QuickGaussianBlurPrivate::rebuildShaders);
    connect(q, &QuickGaussianBlur::heightChanged, this, &QuickGaussianBlurPrivate::rebuildShaders);
    connect(q, &QuickGaussianBlur::radiusChanged, this, &QuickGaussianBlurPrivate::rebuildShaders);
    connect(q, &QuickGaussianBlur::samplesChanged, this, &QuickGaussianBlurPrivate::rebuildShaders);
    connect(q, &QuickGaussianBlur::deviationChanged, this, &QuickGaussianBlurPrivate::rebuildShaders);

    const QRectF sourceRect = {0.0, 0.0, 0.0, 0.0};

    m_shaderBuilder.reset(new QGfxShaderBuilder(q));

    m_sourceProxy.reset(new QGfxSourceProxy(q));
    m_sourceProxy->setInterpolation(QGfxSourceProxy::Interpolation::Linear);
    m_sourceProxy->setSourceRect(sourceRect);
    connect(m_sourceProxy.get(), &QGfxSourceProxy::outputChanged, this, &QuickGaussianBlurPrivate::rebuildShaders);

    m_horizontalBlur.reset(new QQuickShaderEffect(q));
    const auto horizontalBlurAnchors = new QQuickAnchors(m_horizontalBlur.get(), m_horizontalBlur.get());
    horizontalBlurAnchors->setFill(q);
    QQuickItemLayer * const horizontalBlurLayer = QQuickItemPrivate::get(m_horizontalBlur.get())->layer();
    horizontalBlurLayer->setSmooth(true);
    horizontalBlurLayer->setSourceRect(sourceRect);
    horizontalBlurLayer->setEnabled(true);
    m_horizontalBlur->setVisible(false);
    m_horizontalBlur->setBlending(false);

    m_verticalBlur.reset(new QQuickShaderEffect(q));
    const auto verticalBlurAnchors = new QQuickAnchors(m_verticalBlur.get(), m_verticalBlur.get());
    verticalBlurAnchors->setFill(q);
    m_verticalBlur->setVisible(true);

    m_cacheItem.reset(new QQuickShaderEffectSource(q));
    const auto cacheItemAnchors = new QQuickAnchors(m_cacheItem.get(), m_cacheItem.get());
    cacheItemAnchors->setFill(m_verticalBlur.get());
    m_cacheItem->setSmooth(true);
    m_cacheItem->setSourceItem(m_verticalBlur.get());
    m_cacheItem->setLive(true);
    m_cacheItem->setHideSource(m_cached);
    m_cacheItem->setVisible(m_cached);

    rebuildShaders();
}

QuickGaussianBlur::QuickGaussianBlur(QQuickItem *parent) : QQuickItem(parent), d_ptr(new QuickGaussianBlurPrivate(this))
{
}

QuickGaussianBlur::~QuickGaussianBlur() = default;

QQuickItem *QuickGaussianBlur::source() const
{
    Q_D(const QuickGaussianBlur);
    return d->m_source;
}

void QuickGaussianBlur::setSource(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_D(QuickGaussianBlur);
    if (d->m_source == item) {
        return;
    }
    d->m_source = item;
    d->m_sourceProxy->setInput(d->m_source);
    Q_EMIT sourceChanged();
}

qreal QuickGaussianBlur::radius() const
{
    Q_D(const QuickGaussianBlur);
    return d->m_radius;
}

void QuickGaussianBlur::setRadius(const qreal value)
{
    Q_D(QuickGaussianBlur);
    if (qFuzzyCompare(d->m_radius, value)) {
        return;
    }
    d->m_radius = value;
    Q_EMIT radiusChanged();
}

int QuickGaussianBlur::samples() const
{
    Q_D(const QuickGaussianBlur);
    return d->m_samples;
}

void QuickGaussianBlur::setSamples(const int value)
{
    Q_D(QuickGaussianBlur);
    if (d->m_samples == value) {
        return;
    }
    d->m_samples = value;
    Q_EMIT samplesChanged();
}

qreal QuickGaussianBlur::deviation() const
{
    Q_D(const QuickGaussianBlur);
    return d->m_deviation;
}

void QuickGaussianBlur::setDeviation(const qreal value)
{
    Q_D(QuickGaussianBlur);
    if (qFuzzyCompare(d->m_deviation, value)) {
        return;
    }
    d->m_deviation = value;
    Q_EMIT deviationChanged();
}

bool QuickGaussianBlur::isCached() const
{
    Q_D(const QuickGaussianBlur);
    return d->m_cached;
}

void QuickGaussianBlur::setCached(const bool value)
{
    Q_D(QuickGaussianBlur);
    if (d->m_cached == value) {
        return;
    }
    d->m_cached = value;
    d->m_cacheItem->setHideSource(d->m_cached);
    d->m_cacheItem->setVisible(d->m_cached);
    Q_EMIT cachedChanged();
}

void QuickGaussianBlur::itemChange(const ItemChange change, const ItemChangeData &value)
{
    QQuickItem::itemChange(change, value);
    if (change == ItemDevicePixelRatioHasChanged) {
        Q_D(QuickGaussianBlur);
        d->updateDpr(value.realValue);
    }
}
