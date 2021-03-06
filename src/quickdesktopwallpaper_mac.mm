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
#include "quickacrylicmaterial_p.h"
#include <AppKit/AppKit.h>

static inline void initResource()
{
    Q_INIT_RESOURCE(qtacrylicmaterial_mac);
}

void QuickDesktopWallpaperPrivate::subscribeWallpaperChangeNotification_platform()
{
    // ### TODO
}

QString QuickDesktopWallpaperPrivate::getWallpaperImageFilePath()
{
#if 0
    const NSWorkspace * const sharedWorkspace = [NSWorkspace sharedWorkspace];
    if (!sharedWorkspace) {
        qWarning() << "Failed to retrieve the shared workspace.";
        return {};
    }
    NSScreen * const mainScreen = [NSScreen mainScreen];
    if (!mainScreen) {
        qWarning() << "Failed to retrieve the main screen.";
        return {};
    }
    const NSURL * const url = [sharedWorkspace desktopImageURLForScreen:mainScreen];
    if (!url) {
        qWarning() << "Failed to retrieve the desktop image URL.";
        return {};
    }
    const QUrl path = QUrl::fromNSURL(url);
    if (!path.isValid()) {
        qWarning() << "The converted QUrl is not valid.";
        return {};
    }
    return path.toLocalFile();
#else
    initResource();
    const QString theme = (QuickAcrylicMaterialPrivate::shouldAppsUseDarkMode() ? u"Dark"_qs : u"Light"_qs);
    return (u":/org/wangwenx190/QtAcrylicMaterial/assets/Monterey-%1.jpg"_qs).arg(theme);
#endif
}

QuickDesktopWallpaperPrivate::WallpaperImageAspectStyle QuickDesktopWallpaperPrivate::getWallpaperImageAspectStyle()
{
    return WallpaperImageAspectStyle::Stretch;
}
