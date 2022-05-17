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

#include "quickacrylicmaterial.h"
#include "quickacrylicmaterial_p.h"
#include <QtCore/qdebug.h>
#include <QtCore/qmutex.h>
#include <QtCore/qabstractnativeeventfilter.h>
#include <QtCore/qt_windows.h>

class AcrylicMaterialWin32EventFilter;

struct AcrylicMaterialWin32Helper
{
    QMutex mutex;
    QScopedPointer<AcrylicMaterialWin32EventFilter> eventFilter;
    QList<QPointer<QuickAcrylicMaterial>> items = {};
};

Q_GLOBAL_STATIC(AcrylicMaterialWin32Helper, g_acrylicMaterialWin32Helper)

class AcrylicMaterialWin32EventFilter : public QAbstractNativeEventFilter
{
    Q_DISABLE_COPY_MOVE(AcrylicMaterialWin32EventFilter)

public:
    explicit AcrylicMaterialWin32EventFilter() : QAbstractNativeEventFilter() {}
    ~AcrylicMaterialWin32EventFilter() override = default;

    [[nodiscard]] bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        if ((eventType != "windows_generic_MSG"_qba) || !message || !result) {
            return false;
        }
        const auto msg = static_cast<LPMSG>(message);
        if (!msg->hwnd) {
            return false;
        }
        if ((msg->message == WM_SETTINGCHANGE) && (msg->lParam != 0) &&
               (std::wcscmp(reinterpret_cast<LPCWSTR>(msg->lParam), L"ImmersiveColorSet") == 0)) {
            qDebug() << "Detected system theme change event.";
            QMutexLocker locker(&g_acrylicMaterialWin32Helper()->mutex);
            if (!g_acrylicMaterialWin32Helper()->items.isEmpty()) {
                for (auto &&item : qAsConst(g_acrylicMaterialWin32Helper()->items)) {
                    if (item) {
                        if (item->theme() == QuickAcrylicMaterial::Theme::System) {
                            item->setTheme(QuickAcrylicMaterial::Theme::System);
                        }
                    }
                }
            }
        }
        return false;
    }
};

void QuickAcrylicMaterialPrivate::subscribeSystemThemeChangeNotification()
{
    Q_Q(QuickAcrylicMaterial);
    QMutexLocker locker(&g_acrylicMaterialWin32Helper()->mutex);
    if (!g_acrylicMaterialWin32Helper()->items.contains(q)) {
        g_acrylicMaterialWin32Helper()->items.append(q);
    }
    if (g_acrylicMaterialWin32Helper()->eventFilter.isNull()) {
        g_acrylicMaterialWin32Helper()->eventFilter.reset(new AcrylicMaterialWin32EventFilter);
        qApp->installNativeEventFilter(g_acrylicMaterialWin32Helper()->eventFilter.get());
    }
}
