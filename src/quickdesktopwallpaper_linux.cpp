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

#define QT_NO_KEYWORDS

#include "quickdesktopwallpaper.h"
#include "quickdesktopwallpaper_p.h"
#include <gtk/gtk.h>
#include <glib.h>
#include <gconf/gconf-client.h>

void QuickDesktopWallpaperPrivate::subscribeWallpaperChangeNotification_platform()
{
    // ### TODO
}

QString QuickDesktopWallpaperPrivate::getWallpaperImageFilePath()
{
    GConfClient *client = gconf_client_get_default();
    gchar *rawPath = gconf_client_get_string(client, "/desktop/gnome/background/picture_filename", nullptr);
    if (!rawPath) {
        qWarning() << "Failed to retrieve the wallpaper file path.";
        return {};
    }
    return QString::fromUtf8(rawPath);
}

QuickDesktopWallpaperPrivate::WallpaperImageAspectStyle QuickDesktopWallpaperPrivate::getWallpaperImageAspectStyle()
{
    static constexpr const auto defaultAspectStyle = WallpaperImageAspectStyle::Fill;
    GConfClient *client = gconf_client_get_default();
    gchar *rawOptions = gconf_client_get_string(client, "/desktop/gnome/background/picture_options", nullptr);
    if (!rawOptions) {
        qWarning() << "Failed to retrieve the wallpaper tile options.";
        return defaultAspectStyle;
    }
    const QString options = QString::fromUtf8(rawOptions);
    if ((options == u"wallpaper"_qs) || (options == u"tiled"_qs)) {
        return WallpaperImageAspectStyle::Tile;
    }
    if (options == u"centered"_qs) {
        return WallpaperImageAspectStyle::Center;
    }
    if (options == u"stretched"_qs) {
        return WallpaperImageAspectStyle::Stretch;
    }
    if (options == u"scaled"_qs) {
        return WallpaperImageAspectStyle::Fit;
    }
    return defaultAspectStyle;
}
