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
