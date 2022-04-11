#include "quickacrylicitem.h"
#include "quickacrylicitem_p.h"
#include "quickgaussianblur.h"
#include <QtQuick/qquickwindow.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <QtQuick/private/qquickimage_p.h>

static constexpr const QColor sc_defaultTintColor = { 255, 255, 255, 204 };
static constexpr const qreal sc_defaultTintOpacity = 1.0;
static constexpr const qreal sc_defaultBlurRadius = 30.0;
static constexpr const qreal sc_defaultNoiseOpacity = 0.02;
static constexpr const QColor sc_defaultExclusionColor = { 255, 255, 255, 26 };
static constexpr const qreal sc_defaultSaturation = 1.25;

QuickAcrylicItemPrivate::QuickAcrylicItemPrivate(QuickAcrylicItem *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

QuickAcrylicItemPrivate::~QuickAcrylicItemPrivate() = default;

void QuickAcrylicItemPrivate::updateBackgroundSource()
{
    Q_ASSERT(m_backgroundImage);
    if (!m_backgroundImage) {
        return;
    }
    m_backgroundImage->setSource(QUrl(u"qrc:///assets/win11-light.jpg"_qs));
}

void QuickAcrylicItemPrivate::updateBackgroundClipRect()
{
    Q_ASSERT(m_backgroundImage);
    if (!m_backgroundImage) {
        return;
    }
    Q_Q(QuickAcrylicItem);
    m_backgroundImage->setSourceClipRect(QRectF(q->mapToGlobal(QPointF(0.0, 0.0)), q->size()));
}

void QuickAcrylicItemPrivate::createBackgroundImage()
{
    Q_Q(QuickAcrylicItem);
    m_backgroundImage.reset(new QQuickImage(q));
    m_backgroundImage->setClip(true);
    m_backgroundImage->setFillMode(QQuickImage::Pad);
    m_backgroundImage->setHorizontalAlignment(QQuickImage::AlignLeft);
    m_backgroundImage->setVerticalAlignment(QQuickImage::AlignTop);
    m_backgroundImage->setMipmap(true);
    m_backgroundImage->setSmooth(true);
    m_backgroundImage->setVisible(false);
    const auto backgroundImageAnchors = new QQuickAnchors(m_backgroundImage.get(), m_backgroundImage.get());
    backgroundImageAnchors->setFill(q);

    connect(q, &QuickAcrylicItem::widthChanged, this, &QuickAcrylicItemPrivate::updateBackgroundClipRect);
    connect(q, &QuickAcrylicItem::heightChanged, this, &QuickAcrylicItemPrivate::updateBackgroundClipRect);
    connect(q, &QuickAcrylicItem::windowChanged, this, [this](QQuickWindow *window){
        if (m_rootWindowXChangedConnection) {
            disconnect(m_rootWindowXChangedConnection);
            m_rootWindowXChangedConnection = {};
        }
        if (m_rootWindowYChangedConnection) {
            disconnect(m_rootWindowYChangedConnection);
            m_rootWindowYChangedConnection = {};
        }
        if (window) {
            m_rootWindowXChangedConnection = connect(window, &QQuickWindow::xChanged, this, &QuickAcrylicItemPrivate::updateBackgroundClipRect);
            m_rootWindowYChangedConnection = connect(window, &QQuickWindow::yChanged, this, &QuickAcrylicItemPrivate::updateBackgroundClipRect);
        }
    });

    updateBackgroundSource();
    updateBackgroundClipRect();
}

void QuickAcrylicItemPrivate::createBlurredSource()
{
    Q_Q(QuickAcrylicItem);
    m_blurredSource.reset(new QuickGaussianBlur(q));
    m_blurredSource->setSource(m_backgroundImage.get());
    const auto blurredSourceAnchors = new QQuickAnchors(m_blurredSource.get(), m_blurredSource.get());
    blurredSourceAnchors->setFill(q);
}

void QuickAcrylicItemPrivate::initialize()
{
    Q_Q(QuickAcrylicItem);
    q->setClip(true);
    createBackgroundImage();
    createBlurredSource();
}

QuickAcrylicItem::QuickAcrylicItem(QQuickItem *parent) : QQuickItem(parent), d_ptr(new QuickAcrylicItemPrivate(this))
{
}

QuickAcrylicItem::~QuickAcrylicItem() = default;
