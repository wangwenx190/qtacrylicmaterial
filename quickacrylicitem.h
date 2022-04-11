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

public:
    explicit QuickAcrylicItem(QQuickItem *parent = nullptr);
    ~QuickAcrylicItem() override;

private:
    QScopedPointer<QuickAcrylicItemPrivate> d_ptr;
};

QML_DECLARE_TYPE(QuickAcrylicItem)
