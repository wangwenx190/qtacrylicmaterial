#pragma once

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE
class QQuickImage;
QT_END_NAMESPACE

class QuickAcrylicItem;
class QuickGaussianBlur;

class QuickAcrylicItemPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QuickAcrylicItem)
    Q_DISABLE_COPY_MOVE(QuickAcrylicItemPrivate)

public:
    explicit QuickAcrylicItemPrivate(QuickAcrylicItem *q);
    ~QuickAcrylicItemPrivate() override;

public Q_SLOTS:
    void updateBackgroundSource();
    void updateBackgroundClipRect();

private:
    void createBackgroundImage();
    void createBlurredSource();
    void initialize();

private:
    QuickAcrylicItem *q_ptr = nullptr;
    QScopedPointer<QQuickImage> m_backgroundImage;
    QMetaObject::Connection m_rootWindowXChangedConnection = {};
    QMetaObject::Connection m_rootWindowYChangedConnection = {};
    QScopedPointer<QuickGaussianBlur> m_blurredSource;
};
