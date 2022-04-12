#pragma once

#include <QtCore/qobject.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE
class QQuickImage;
class QQuickRectangle;
QT_END_NAMESPACE

class QuickAcrylicItem;
class QuickGaussianBlur;
class QuickBlend;

class QuickAcrylicItemPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QuickAcrylicItem)
    Q_DISABLE_COPY_MOVE(QuickAcrylicItemPrivate)

public:
    explicit QuickAcrylicItemPrivate(QuickAcrylicItem *q);
    ~QuickAcrylicItemPrivate() override;

    [[nodiscard]] static QuickAcrylicItemPrivate *get(QuickAcrylicItem *pub);
    [[nodiscard]] static const QuickAcrylicItemPrivate *get(const QuickAcrylicItem *pub);

public Q_SLOTS:
    void updateBackgroundSource();
    void updateBackgroundClipRect();

private:
    void createBackgroundImage();
    void createBlurredSource();
    void createLuminosityColorEffect();
    void createLuminosityBlendEffect();
    void createTintColorEffect();
    void createTintBlendEffect();
    void createNoiseBorderEffect();
    void createNoiseBlendEffect();
    void initialize();
    [[nodiscard]] qreal calculateTintOpacityModifier(const QColor &tintColor) const;
    [[nodiscard]] QColor calculateLuminosityColor(const QColor &tintColor, const std::optional<qreal> luminosityOpacity) const;
    [[nodiscard]] QColor calculateEffectiveTintColor() const;
    [[nodiscard]] QColor calculateEffectiveLuminosityColor() const;

private:
    QuickAcrylicItem *q_ptr = nullptr;
    QColor m_tintColor = {};
    qreal m_tintOpacity = 0.0;
    std::optional<qreal> m_luminosityOpacity = std::nullopt;
    qreal m_noiseOpacity = 0.0;
    QMetaObject::Connection m_rootWindowXChangedConnection = {};
    QMetaObject::Connection m_rootWindowYChangedConnection = {};
    QScopedPointer<QQuickImage> m_backgroundImage;
    QScopedPointer<QuickGaussianBlur> m_blurredSource;
    QScopedPointer<QQuickRectangle> m_luminosityColorEffect;
    QScopedPointer<QuickBlend> m_luminosityBlendEffect;
    QScopedPointer<QQuickRectangle> m_tintColorEffect;
    QScopedPointer<QuickBlend> m_tintBlendEffect;
    QScopedPointer<QQuickImage> m_noiseBorderEffect;
    QScopedPointer<QuickBlend> m_noiseBlendEffect;
};
