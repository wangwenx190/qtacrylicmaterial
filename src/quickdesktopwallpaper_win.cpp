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
#include <QtCore/private/qwinregistry_p.h>
#include <QtCore/qt_windows.h>

using WallpaperImageAspectStyle = QuickDesktopWallpaper::WallpaperImageAspectStyle;

[[nodiscard]] QString getWallpaperImageFilePath()
{
    wchar_t path[MAX_PATH] = {};
    if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, path, 0) == FALSE) {
        return {};
    }
    return QString::fromWCharArray(path);
}

[[nodiscard]] WallpaperImageAspectStyle getWallpaperImageAspectStyle()
{
    static constexpr const auto defaultStyle = WallpaperImageAspectStyle::KeepRatioByExpanding;
    const QWinRegistryKey desktopSettings(HKEY_CURRENT_USER, uR"(Control Panel\Desktop)");
    if (!desktopSettings.isValid()) {
        return defaultStyle;
    }
    const QPair<DWORD, bool> wallpaperStyle = desktopSettings.dwordValue(u"WallpaperStyle");
    if (!wallpaperStyle.second) {
        return defaultStyle;
    }
    switch (wallpaperStyle.first) {
    case 0: {
        const QPair<DWORD, bool> tileWallpaper = desktopSettings.dwordValue(u"TileWallpaper");
        if (tileWallpaper.second && (tileWallpaper.first != 0)) {
            return WallpaperImageAspectStyle::Tiled;
        }
        return WallpaperImageAspectStyle::Central;
    }
    case 2:
        return WallpaperImageAspectStyle::IgnoreRatio;
    case 6:
        return WallpaperImageAspectStyle::KeepRatio;
    default:
        return defaultStyle;
    }
}
