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
#include <QtCore/qfileinfo.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/private/qquickimage_p.h>
#include <QtQuick/private/qquickanchors_p.h>

[[nodiscard]] extern QString getWallpaperImageFilePath();
[[nodiscard]] extern QuickDesktopWallpaper::WallpaperImageAspectStyle getWallpaperImageAspectStyle();

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

void QuickDesktopWallpaperPrivate::updateWallpaperSource()
{
    const QString filePath = getWallpaperImageFilePath();
    if (filePath.isEmpty()) {
        return;
    }
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return;
    }
    // ### TODO: check is actually a valid bitmap file or not.
    if (m_wallpaperFilePath == filePath) {
        return;
    }
    m_wallpaperFilePath = filePath;
    m_wallpaperImage->setSource(QUrl::fromLocalFile(m_wallpaperFilePath));
    m_wallpaperImage->setSourceSize(QImage(m_wallpaperFilePath).size());
}

void QuickDesktopWallpaperPrivate::updateWallpaperAspectStyle()
{
    const WallpaperImageAspectStyle style = getWallpaperImageAspectStyle();
    if (m_wallpaperAspectStyle == style) {
        return;
    }
    m_wallpaperAspectStyle = style;
    switch (m_wallpaperAspectStyle) {
    case WallpaperImageAspectStyle::Central:
        m_wallpaperImage->setFillMode(QQuickImage::Pad);
        break;
    case WallpaperImageAspectStyle::Tiled:
        m_wallpaperImage->setFillMode(QQuickImage::Tile);
        break;
    case WallpaperImageAspectStyle::IgnoreRatio:
        m_wallpaperImage->setFillMode(QQuickImage::Stretch);
        break;
    case WallpaperImageAspectStyle::KeepRatio:
        m_wallpaperImage->setFillMode(QQuickImage::PreserveAspectFit);
        break;
    case WallpaperImageAspectStyle::KeepRatioByExpanding:
        m_wallpaperImage->setFillMode(QQuickImage::PreserveAspectCrop);
        break;
    }
}

void QuickDesktopWallpaperPrivate::updateWallpaperClipRect()
{
    const QRectF rect = {m_wallpaperImage->mapToGlobal(QPointF(0.0, 0.0)), m_wallpaperImage->size()};
    if (m_wallpaperClipRect == rect) {
        return;
    }
    m_wallpaperClipRect = rect;
    m_wallpaperImage->setSourceClipRect(m_wallpaperClipRect);
}

void QuickDesktopWallpaperPrivate::initialize()
{
    Q_Q(QuickDesktopWallpaper);
    q->setClip(true);
    m_wallpaperImage.reset(new QQuickImage(q));
    m_wallpaperImage->setSmooth(true);
    m_wallpaperImage->setMipmap(true);
    const auto wallpaperImageAnchors = new QQuickAnchors(m_wallpaperImage.get(), m_wallpaperImage.get());
    wallpaperImageAnchors->setFill(q);

    connect(q, &QuickDesktopWallpaper::widthChanged, this, &QuickDesktopWallpaperPrivate::updateWallpaperClipRect);
    connect(q, &QuickDesktopWallpaper::heightChanged, this, &QuickDesktopWallpaperPrivate::updateWallpaperClipRect);
    connect(q, &QuickDesktopWallpaper::windowChanged, this, [this](QQuickWindow *window){
        if (m_rootWindowXChangedConnection) {
            disconnect(m_rootWindowXChangedConnection);
            m_rootWindowXChangedConnection = {};
        }
        if (m_rootWindowYChangedConnection) {
            disconnect(m_rootWindowYChangedConnection);
            m_rootWindowYChangedConnection = {};
        }
        if (!window) {
            return;
        }
        m_rootWindowXChangedConnection = connect(window, &QQuickWindow::xChanged, this, &QuickDesktopWallpaperPrivate::updateWallpaperClipRect);
        m_rootWindowYChangedConnection = connect(window, &QQuickWindow::yChanged, this, &QuickDesktopWallpaperPrivate::updateWallpaperClipRect);
    });

    updateWallpaperSource();
    updateWallpaperAspectStyle();
    updateWallpaperClipRect();
}

QuickDesktopWallpaper::QuickDesktopWallpaper(QQuickItem *parent) : QQuickItem(parent), d_ptr(new QuickDesktopWallpaperPrivate(this))
{
}

QuickDesktopWallpaper::~QuickDesktopWallpaper() = default;
