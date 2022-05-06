/****************************************************************************
**
** Copyright (C) 2022 Jolla Ltd, author: <gunnar.sletta@jollamobile.com>
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Graphical Effects module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#pragma once

#include <QtQml/qqmlregistration.h>
#include <QtQuick/qquickitem.h>

QT_BEGIN_NAMESPACE

class QQuickShaderEffectSource;

class QGfxSourceProxy : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SourceProxy)
    Q_DISABLE_COPY_MOVE(QGfxSourceProxy)

    Q_PROPERTY(QQuickItem *input READ input WRITE setInput NOTIFY inputChanged RESET resetInput FINAL)
    Q_PROPERTY(QQuickItem *output READ output NOTIFY outputChanged FINAL)
    Q_PROPERTY(QRectF sourceRect READ sourceRect WRITE setSourceRect NOTIFY sourceRectChanged FINAL)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged FINAL)
    Q_PROPERTY(Interpolation interpolation READ interpolation WRITE setInterpolation NOTIFY interpolationChanged FINAL)

public:
    enum class Interpolation
    {
        Any, Nearest, Linear
    };
    Q_ENUM(Interpolation)

    explicit QGfxSourceProxy(QQuickItem *parent = nullptr);
    ~QGfxSourceProxy() override;

    [[nodiscard]] QQuickItem *input() const { return m_input; }
    void setInput(QQuickItem *input);
    void resetInput() { setInput(nullptr); }

    [[nodiscard]] QQuickItem *output() const { return m_output; }

    [[nodiscard]] QRectF sourceRect() const { return m_sourceRect; }
    void setSourceRect(const QRectF &sourceRect);

    [[nodiscard]] bool isActive() const { return (m_output && (m_output != m_input)); }

    void setInterpolation(const Interpolation i);
    [[nodiscard]] Interpolation interpolation() const { return m_interpolation; }

protected:
    void updatePolish() override;

Q_SIGNALS:
    void inputChanged();
    void outputChanged();
    void sourceRectChanged();
    void activeChanged();
    void interpolationChanged();

private Q_SLOTS:
    void repolish();

private:
    void setOutput(QQuickItem *output);
    void useProxy();
    [[nodiscard]] static QObject *findLayer(QQuickItem *item);

private:
    QRectF m_sourceRect = {};
    QQuickItem *m_input = nullptr;
    QQuickItem *m_output = nullptr;
    QScopedPointer<QQuickShaderEffectSource> m_proxy;
    Interpolation m_interpolation = Interpolation::Any;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QT_PREPEND_NAMESPACE(QGfxSourceProxy))
