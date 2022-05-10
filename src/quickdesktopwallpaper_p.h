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

class QuickDesktopWallpaper;

class QuickDesktopWallpaperPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QuickDesktopWallpaper)
    Q_DISABLE_COPY_MOVE(QuickDesktopWallpaperPrivate)

public:
    enum class WallpaperImageAspectStyle
    {
        Fill, // Keep aspect ratio to fill, expand/crop if necessary.
        Fit, // Keep aspect ratio to fill, but don't expand/crop.
        Stretch, // Ignore aspect ratio to fill.
        Tile,
        Center,
        Span // ???
    };
    Q_ENUM(WallpaperImageAspectStyle)

    explicit QuickDesktopWallpaperPrivate(QuickDesktopWallpaper *q);
    ~QuickDesktopWallpaperPrivate() override;

    [[nodiscard]] static QuickDesktopWallpaperPrivate *get(QuickDesktopWallpaper *pub);
    [[nodiscard]] static const QuickDesktopWallpaperPrivate *get(const QuickDesktopWallpaper *pub);

    [[nodiscard]] static QString getWallpaperImageFilePath();
    [[nodiscard]] static WallpaperImageAspectStyle getWallpaperImageAspectStyle();

    static void subscribeWallpaperChangeNotification(QObject *object);

public Q_SLOTS:
    void rebindWindow();

private:
    void initialize();

private:
    QuickDesktopWallpaper *q_ptr = nullptr;
    QMetaObject::Connection m_rootWindowXChangedConnection = {};
    QMetaObject::Connection m_rootWindowYChangedConnection = {};
};
