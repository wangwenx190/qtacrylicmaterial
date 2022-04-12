#pragma once

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

class QuickGaussianBlurPrivate : public QObject
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
    void updateDpr(const QScreen *screen);

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
    QMetaObject::Connection m_updateDprConnection = {};
    QScopedPointer<QGfxShaderBuilder> m_shaderBuilder;
    QScopedPointer<QGfxSourceProxy> m_sourceProxy;
    QScopedPointer<QQuickShaderEffect> m_horizontalBlur;
    QScopedPointer<QQuickShaderEffect> m_verticalBlur;
    QScopedPointer<QQuickShaderEffectSource> m_cacheItem;
};
