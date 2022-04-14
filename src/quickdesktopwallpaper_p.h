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

#pragma once

#include <QtCore/qobject.h>
#include "quickdesktopwallpaper.h"

QT_BEGIN_NAMESPACE
class QQuickImage;
QT_END_NAMESPACE

class QuickDesktopWallpaperPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QuickDesktopWallpaper)
    Q_DISABLE_COPY_MOVE(QuickDesktopWallpaperPrivate)

public:
    using WallpaperImageAspectStyle = QuickDesktopWallpaper::WallpaperImageAspectStyle;

    explicit QuickDesktopWallpaperPrivate(QuickDesktopWallpaper *q);
    ~QuickDesktopWallpaperPrivate() override;

    [[nodiscard]] static QuickDesktopWallpaperPrivate *get(QuickDesktopWallpaper *pub);
    [[nodiscard]] static const QuickDesktopWallpaperPrivate *get(const QuickDesktopWallpaper *pub);

public Q_SLOTS:
    void updateWallpaperSource();
    void updateWallpaperAspectStyle();
    void updateWallpaperClipRect();

private:
    void initialize();

private:
    QuickDesktopWallpaper *q_ptr = nullptr;
    QString m_wallpaperFilePath = {};
    WallpaperImageAspectStyle m_wallpaperAspectStyle = WallpaperImageAspectStyle::KeepRatioByExpanding;
    QRectF m_wallpaperClipRect = {};
    QScopedPointer<QQuickImage> m_wallpaperImage;
    QMetaObject::Connection m_rootWindowXChangedConnection = {};
    QMetaObject::Connection m_rootWindowYChangedConnection = {};
};