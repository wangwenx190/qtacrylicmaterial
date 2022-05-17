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
#include <QtCore/qdebug.h>
#include <QtCore/qmutex.h>
#include <QtCore/qabstractnativeeventfilter.h>
#include <QtCore/private/qwinregistry_p.h>
#include <QtCore/qt_windows.h>

class DesktopWallpaperWin32EventFilter;

struct DesktopWallpaperWin32Helper
{
    QMutex mutex;
    QScopedPointer<DesktopWallpaperWin32EventFilter> eventFilter;
    QList<QPointer<QuickDesktopWallpaper>> items = {};
};

Q_GLOBAL_STATIC(DesktopWallpaperWin32Helper, g_desktopWallpaperWin32Helper)

class DesktopWallpaperWin32EventFilter : public QAbstractNativeEventFilter
{
    Q_DISABLE_COPY_MOVE(DesktopWallpaperWin32EventFilter)

public:
    explicit DesktopWallpaperWin32EventFilter() : QAbstractNativeEventFilter() {}
    ~DesktopWallpaperWin32EventFilter() override = default;

    [[nodiscard]] bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        if ((eventType != "windows_generic_MSG"_qba) || !message || !result) {
            return false;
        }
        const auto msg = static_cast<LPMSG>(message);
        if (!msg->hwnd) {
            return false;
        }
        if ((msg->message == WM_SETTINGCHANGE) && (msg->wParam == SPI_SETDESKWALLPAPER)) {
            qDebug() << "Detected desktop wallpaper change event.";
            QMutexLocker locker(&g_desktopWallpaperWin32Helper()->mutex);
            if (!g_desktopWallpaperWin32Helper()->items.isEmpty()) {
                for (auto &&item : qAsConst(g_desktopWallpaperWin32Helper()->items)) {
                    if (item) {
                        QuickDesktopWallpaperPrivate::get(item)->forceRegenerateWallpaperImageCache();
                        item->update(); // Force re-paint immediately.
                    }
                }
            }
        }
        return false;
    }
};

void QuickDesktopWallpaperPrivate::subscribeWallpaperChangeNotification_platform()
{
    Q_Q(QuickDesktopWallpaper);
    QMutexLocker locker(&g_desktopWallpaperWin32Helper()->mutex);
    if (!g_desktopWallpaperWin32Helper()->items.contains(q)) {
        g_desktopWallpaperWin32Helper()->items.append(q);
    }
    if (g_desktopWallpaperWin32Helper()->eventFilter.isNull()) {
        g_desktopWallpaperWin32Helper()->eventFilter.reset(new DesktopWallpaperWin32EventFilter);
        qApp->installNativeEventFilter(g_desktopWallpaperWin32Helper()->eventFilter.get());
    }
}

QString QuickDesktopWallpaperPrivate::getWallpaperImageFilePath()
{
    wchar_t path[MAX_PATH] = {};
    if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, path, 0) == FALSE) {
        qWarning() << "Failed to retrieve the desktop wallpaper file path.";
        return {};
    }
    return QString::fromWCharArray(path);
}

QuickDesktopWallpaperPrivate::WallpaperImageAspectStyle QuickDesktopWallpaperPrivate::getWallpaperImageAspectStyle()
{
    static constexpr const auto defaultStyle = WallpaperImageAspectStyle::Fill;
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
            return WallpaperImageAspectStyle::Tile;
        }
        return WallpaperImageAspectStyle::Center;
    }
    case 2:
        return WallpaperImageAspectStyle::Stretch; // Ignore aspect ratio to fill.
    case 6:
        return WallpaperImageAspectStyle::Fit; // Keep aspect ratio to fill, but don't expand/crop.
    case 10:
        return WallpaperImageAspectStyle::Fill; // Keep aspect ratio to fill, expand/crop if necessary.
    case 22:
        return WallpaperImageAspectStyle::Span; // ???
    default:
        return defaultStyle;
    }
}
