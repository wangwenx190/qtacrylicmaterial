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
#include <QtQuick/private/qquickimage_p.h>
#include <QtQuick/private/qquickanchors_p.h>

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

void QuickDesktopWallpaperPrivate::initialize()
{

}

QuickDesktopWallpaper::QuickDesktopWallpaper(QQuickItem *parent) : QQuickItem(parent), d_ptr(new QuickDesktopWallpaperPrivate(this))
{
}

QuickDesktopWallpaper::~QuickDesktopWallpaper() = default;
