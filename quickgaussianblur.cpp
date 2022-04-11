#include "quickgaussianblur.h"
#include "quickgaussianblur_p.h"
#include "qgfxsourceproxy_p.h"
#include "qgfxshaderbuilder_p.h"
#include <QtCore/qmath.h>
#include <QtQuick/private/qquickshadereffect_p.h>
#include <QtQuick/private/qquickshadereffectsource_p.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <QtQuick/private/qquickitem_p.h>

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

void QuickGaussianBlurPrivate::rebuildShaders()
{
    QJSValue params = {};
    params.setProperty(u"radius"_qs, m_kernelRadius);
    // Limit deviation to something very small avoid getting NaN in the shader.
    params.setProperty(u"deviation"_qs, qMax(0.00001, m_deviation));
    params.setProperty(u"alphaOnly"_qs, m_alphaOnly);
    params.setProperty(u"masked"_qs, false);
    params.setProperty(u"fallback"_qs, !qFuzzyCompare(m_radius, m_kernelRadius));
    const QVariantMap shaders = m_shaderBuilder->gaussianBlur(params);
    m_horizontalBlur->setFragmentShader(shaders.value(u"fragmentShader"_qs).toUrl());
    m_horizontalBlur->setVertexShader(shaders.value(u"vertexShader"_qs).toUrl());
    m_verticalBlur->setFragmentShader(m_horizontalBlur->fragmentShader());
    m_verticalBlur->setVertexShader(m_horizontalBlur->vertexShader());
}

void QuickGaussianBlurPrivate::recalculateBlurParameters()
{
    m_kernelRadius = qMax(0.0, (qreal(m_samples) / 2.0));
    m_kernelSize = int(qRound((m_kernelRadius * 2.0) + 1.0));
}

void QuickGaussianBlurPrivate::initialize()
{
    Q_Q(QuickGaussianBlur);

    m_samples = 9;
    m_radius = qFloor(qreal(m_samples) / 2.0);
    m_deviation = ((m_radius + 1.0) / 3.3333);

    recalculateBlurParameters();

    const QRectF sourceRect = {0.0, 0.0, 0.0, 0.0};

    m_shaderBuilder.reset(new QGfxShaderBuilder(q));

    m_sourceProxy.reset(new QGfxSourceProxy(q));
    m_sourceProxy->setInterpolation(QGfxSourceProxy::Interpolation::Linear);
    m_sourceProxy->setSourceRect(sourceRect);

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
    if (d->m_radius == value) {
        return;
    }
    d->m_radius = value;
    d->recalculateBlurParameters();
    d->rebuildShaders();
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
    d->recalculateBlurParameters();
    d->rebuildShaders();
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
    if (d->m_deviation == value) {
        return;
    }
    d->m_deviation = value;
    d->recalculateBlurParameters();
    d->rebuildShaders();
    Q_EMIT deviationChanged();
}

bool QuickGaussianBlur::cached() const
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
