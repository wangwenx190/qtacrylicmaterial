/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "quickdesktopwallpaper.h"
#include "quickdesktopwallpaper_p.h"
#include <QtCore/qmutex.h>
#include <QtGui/qscreen.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpainter.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qsgsimpletexturenode.h>

struct InternalHelper
{
    QMutex mutex;
    QPixmap pixmap;
};

Q_GLOBAL_STATIC(InternalHelper, g_helper)

/*!
    Transforms an \a alignment of Qt::AlignLeft or Qt::AlignRight
    without Qt::AlignAbsolute into Qt::AlignLeft or Qt::AlignRight with
    Qt::AlignAbsolute according to the layout \a direction. The other
    alignment flags are left untouched.

    If no horizontal alignment was specified, the function returns the
    default alignment for the given layout \a direction.

    QWidget::layoutDirection
*/
[[nodiscard]] static inline Qt::Alignment visualAlignment(const Qt::LayoutDirection direction, const Qt::Alignment alignment)
{
    return QGuiApplicationPrivate::visualAlignment(direction, alignment);
}

/*!
    Returns a new rectangle of the specified \a size that is aligned to the given \a
    rectangle according to the specified \a alignment and \a direction.
 */
[[nodiscard]] static inline QRect alignedRect(const Qt::LayoutDirection direction, const Qt::Alignment alignment, const QSize &size, const QRect &rectangle)
{
    const Qt::Alignment align = visualAlignment(direction, alignment);
    int x = rectangle.x();
    int y = rectangle.y();
    const int w = size.width();
    const int h = size.height();
    if ((align & Qt::AlignVCenter) == Qt::AlignVCenter) {
        y += ((rectangle.size().height() / 2) - (h / 2));
    } else if ((align & Qt::AlignBottom) == Qt::AlignBottom) {
        y += (rectangle.size().height() - h);
    }
    if ((align & Qt::AlignRight) == Qt::AlignRight) {
        x += (rectangle.size().width() - w);
    } else if ((align & Qt::AlignHCenter) == Qt::AlignHCenter) {
        x += ((rectangle.size().width() / 2) - (w / 2));
    }
    return QRect(x, y, w, h);
}

class WallpaperImageNode : public QObject, public QSGTransformNode
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WallpaperImageNode)

public:
    explicit WallpaperImageNode(QQuickItem *item);
    ~WallpaperImageNode() override;

public Q_SLOTS:
    void maybeGenerateWallpaperImageCache();
    void maybeUpdateWallpaperImageClipRect();
    void forceRegenerateWallpaperImageCache();

private:
    QScopedPointer<QSGTexture> m_texture;
    QPointer<QQuickItem> m_item = nullptr;
    QSGSimpleTextureNode *m_node = nullptr;

    using WallpaperImageAspectStyle = QuickDesktopWallpaperPrivate::WallpaperImageAspectStyle;
};

WallpaperImageNode::WallpaperImageNode(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    m_item = item;

    m_node = new QSGSimpleTextureNode;
    m_node->setFiltering(QSGTexture::Linear);
    maybeGenerateWallpaperImageCache();
    maybeUpdateWallpaperImageClipRect();
    appendChildNode(m_node);

    connect(m_item->window(), &QQuickWindow::beforeRendering, this, &WallpaperImageNode::maybeUpdateWallpaperImageClipRect, Qt::DirectConnection);

    QuickDesktopWallpaperPrivate::subscribeWallpaperChangeNotification(this);
}

WallpaperImageNode::~WallpaperImageNode() = default;

void WallpaperImageNode::maybeGenerateWallpaperImageCache()
{
    QMutexLocker locker(&g_helper()->mutex);
    if (!g_helper()->pixmap.isNull()) {
        return;
    }
    const QSize desktopSize = (m_item->window() ? m_item->window()->screen()->virtualSize()
                               : QGuiApplication::primaryScreen()->virtualSize());
    g_helper()->pixmap = QPixmap(desktopSize);
    g_helper()->pixmap.fill(QColorConstants::Transparent);
    QImage image(QuickDesktopWallpaperPrivate::getWallpaperImageFilePath());
    if (image.isNull()) {
        return;
    }
    const WallpaperImageAspectStyle aspectStyle = QuickDesktopWallpaperPrivate::getWallpaperImageAspectStyle();
    QImage buffer(desktopSize, QImage::Format_ARGB32_Premultiplied);
#ifdef Q_OS_WINDOWS
    if (aspectStyle == WallpaperImageAspectStyle::Center) {
        buffer.fill(QColorConstants::Black);
    }
#endif
    if ((aspectStyle == WallpaperImageAspectStyle::Stretch)
        || (aspectStyle == WallpaperImageAspectStyle::Fit)
        || (aspectStyle == WallpaperImageAspectStyle::Fill)) {
        Qt::AspectRatioMode mode = Qt::KeepAspectRatioByExpanding;
        if (aspectStyle == WallpaperImageAspectStyle::Stretch) {
            mode = Qt::IgnoreAspectRatio;
        } else if (aspectStyle == WallpaperImageAspectStyle::Fit) {
            mode = Qt::KeepAspectRatio;
        }
        QSize newSize = image.size();
        newSize.scale(desktopSize, mode);
        image = image.scaled(newSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    const QRect desktopRect = {QPoint(0, 0), desktopSize};
    if (aspectStyle == WallpaperImageAspectStyle::Tile) {
        QPainter bufferPainter(&buffer);
        const QBrush brush(image);
        bufferPainter.fillRect(desktopRect, brush);
    } else {
        QPainter bufferPainter(&buffer);
        const QRect r = alignedRect(Qt::LeftToRight, Qt::AlignCenter, image.size(), desktopRect);
        bufferPainter.drawImage(r.topLeft(), image);
    }
    QPainter painter(&g_helper()->pixmap);
    painter.drawImage(QPoint(0, 0), buffer);
    m_texture.reset(m_item->window()->createTextureFromImage(g_helper()->pixmap.toImage()));
    m_node->setTexture(m_texture.get());
}

void WallpaperImageNode::maybeUpdateWallpaperImageClipRect()
{
    const QSizeF itemSize = m_item->size();
    m_node->setRect(QRectF(QPointF(0.0, 0.0), itemSize));
    m_node->setSourceRect(QRectF(m_item->mapToGlobal(QPointF(0.0, 0.0)), itemSize));
}

void WallpaperImageNode::forceRegenerateWallpaperImageCache()
{
    g_helper()->mutex.lock();
    g_helper()->pixmap = {};
    g_helper()->mutex.unlock();
    maybeGenerateWallpaperImageCache();
}

QuickDesktopWallpaperPrivate::QuickDesktopWallpaperPrivate(QuickDesktopWallpaper *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

QuickDesktopWallpaperPrivate::~QuickDesktopWallpaperPrivate() = default;

QuickDesktopWallpaperPrivate *QuickDesktopWallpaperPrivate::get(QuickDesktopWallpaper *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const QuickDesktopWallpaperPrivate *QuickDesktopWallpaperPrivate::get(const QuickDesktopWallpaper *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

void QuickDesktopWallpaperPrivate::rebindWindow()
{
    Q_Q(QuickDesktopWallpaper);
    QQuickWindow * const window = q->window();
    if (!window) {
        return;
    }
    if (m_rootWindowXChangedConnection) {
        disconnect(m_rootWindowXChangedConnection);
        m_rootWindowXChangedConnection = {};
    }
    if (m_rootWindowYChangedConnection) {
        disconnect(m_rootWindowYChangedConnection);
        m_rootWindowYChangedConnection = {};
    }
    m_rootWindowXChangedConnection = connect(window, &QQuickWindow::xChanged, q, [q](){ q->update(); });
    m_rootWindowYChangedConnection = connect(window, &QQuickWindow::yChanged, q, [q](){ q->update(); });
}

void QuickDesktopWallpaperPrivate::initialize()
{
    Q_Q(QuickDesktopWallpaper);
    q->setFlag(QuickDesktopWallpaper::ItemHasContents);
    q->setClip(true);
}

QuickDesktopWallpaper::QuickDesktopWallpaper(QQuickItem *parent)
    : QQuickItem(parent), d_ptr(new QuickDesktopWallpaperPrivate(this))
{
}

QuickDesktopWallpaper::~QuickDesktopWallpaper() = default;

void QuickDesktopWallpaper::itemChange(const ItemChange change, const ItemChangeData &value)
{
    QQuickItem::itemChange(change, value);
    Q_D(QuickDesktopWallpaper);
    switch (change) {
    case ItemDevicePixelRatioHasChanged: {
        // TODO
    } break;
    case ItemSceneChange: {
        if (value.window) {
            d->rebindWindow();
        }
    } break;
    default:
        break;
    }
}

QSGNode *QuickDesktopWallpaper::updatePaintNode(QSGNode *old, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    auto node = static_cast<WallpaperImageNode *>(old);
    if (!node) {
        node = new WallpaperImageNode(this);
    }
    return node;
}

#include "quickdesktopwallpaper.moc"
