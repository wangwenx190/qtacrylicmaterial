#pragma once

#include <QtQml/qqmlregistration.h>
#include <QtQuick/qquickitem.h>

class QuickAcrylicItemPrivate;

class QuickAcrylicItem : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(AcrylicItem)
    Q_DECLARE_PRIVATE(QuickAcrylicItem)
    Q_DISABLE_COPY_MOVE(QuickAcrylicItem)

    Q_PROPERTY(QColor tintColor READ tintColor WRITE setTintColor NOTIFY tintColorChanged FINAL)
    Q_PROPERTY(qreal tintOpacity READ tintOpacity WRITE setTintOpacity NOTIFY tintOpacityChanged FINAL)
    Q_PROPERTY(qreal luminosityOpacity READ luminosityOpacity WRITE setLuminosityOpacity NOTIFY luminosityOpacityChanged FINAL)
    Q_PROPERTY(qreal noiseOpacity READ noiseOpacity WRITE setNoiseOpacity NOTIFY noiseOpacityChanged FINAL)

public:
    explicit QuickAcrylicItem(QQuickItem *parent = nullptr);
    ~QuickAcrylicItem() override;

    [[nodiscard]] QColor tintColor() const;
    void setTintColor(const QColor &color);

    [[nodiscard]] qreal tintOpacity() const;
    void setTintOpacity(const qreal opacity);

    [[nodiscard]] qreal luminosityOpacity() const;
    void setLuminosityOpacity(const qreal opacity);

    [[nodiscard]] qreal noiseOpacity() const;
    void setNoiseOpacity(const qreal opacity);

Q_SIGNALS:
    void tintColorChanged();
    void tintOpacityChanged();
    void luminosityOpacityChanged();
    void noiseOpacityChanged();

private:
    QScopedPointer<QuickAcrylicItemPrivate> d_ptr;
};

QML_DECLARE_TYPE(QuickAcrylicItem)
