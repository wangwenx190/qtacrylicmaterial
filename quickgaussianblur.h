#pragma once

#include <QtQml/qqmlregistration.h>
#include <QtQuick/qquickitem.h>

class QuickGaussianBlurPrivate;

class QuickGaussianBlur : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GaussianBlur)
    Q_DECLARE_PRIVATE(QuickGaussianBlur)
    Q_DISABLE_COPY_MOVE(QuickGaussianBlur)

    Q_PROPERTY(QQuickItem* source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged FINAL)
    Q_PROPERTY(int samples READ samples WRITE setSamples NOTIFY samplesChanged FINAL)
    Q_PROPERTY(qreal deviation READ deviation WRITE setDeviation NOTIFY deviationChanged FINAL)
    Q_PROPERTY(bool cached READ isCached WRITE setCached NOTIFY cachedChanged FINAL)

public:
    explicit QuickGaussianBlur(QQuickItem *parent = nullptr);
    ~QuickGaussianBlur() override;

    [[nodiscard]] QQuickItem *source() const;
    void setSource(QQuickItem *item);

    [[nodiscard]] qreal radius() const;
    void setRadius(const qreal value);

    [[nodiscard]] int samples() const;
    void setSamples(const int value);

    [[nodiscard]] qreal deviation() const;
    void setDeviation(const qreal value);

    [[nodiscard]] bool isCached() const;
    void setCached(const bool value);

Q_SIGNALS:
    void sourceChanged();
    void radiusChanged();
    void samplesChanged();
    void deviationChanged();
    void cachedChanged();

private:
    QScopedPointer<QuickGaussianBlurPrivate> d_ptr;
};

QML_DECLARE_TYPE(QuickGaussianBlur)
