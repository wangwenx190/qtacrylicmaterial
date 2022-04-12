#pragma once

#include <QtCore/qobject.h>
#include "quickblend.h"

QT_BEGIN_NAMESPACE
class QGfxSourceProxy;
class QGfxShaderBuilder;
class QQuickShaderEffectSource;
class QQuickShaderEffect;
QT_END_NAMESPACE

class QuickBlend;

class QuickBlendPrivate : public QObject
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
