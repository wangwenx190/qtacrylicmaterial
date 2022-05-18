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

#include "qgfxsourceproxy_p.h"
#include <QtQuick/private/qquickshadereffectsource_p.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickimage_p.h>

QT_BEGIN_NAMESPACE

QGfxSourceProxy::QGfxSourceProxy(QQuickItem *parent) : QQuickItem(parent)
{
    qRegisterMetaType<Interpolation>();
}

QGfxSourceProxy::~QGfxSourceProxy() = default;

void QGfxSourceProxy::setInput(QQuickItem *input)
{
    if (m_input == input) {
        return;
    }
    if (m_input != nullptr) {
        disconnect(m_input, nullptr, this, nullptr);
    }
    m_input = input;
    polish();
    if (m_input != nullptr) {
        if (const auto image = qobject_cast<QQuickImage *>(m_input)) {
            connect(image, &QQuickImage::sourceSizeChanged, this, &QGfxSourceProxy::repolish);
            connect(image, &QQuickImage::fillModeChanged, this, &QGfxSourceProxy::repolish);
        }
        connect(m_input, &QQuickItem::childrenChanged, this, &QGfxSourceProxy::repolish);
    }
    Q_EMIT inputChanged();
}

void QGfxSourceProxy::setOutput(QQuickItem *output)
{
    if (m_output == output) {
        return;
    }
    m_output = output;
    Q_EMIT activeChanged();
    Q_EMIT outputChanged();
}

void QGfxSourceProxy::setSourceRect(const QRectF &sourceRect)
{
    if (m_sourceRect == sourceRect) {
        return;
    }
    m_sourceRect = sourceRect;
    polish();
    Q_EMIT sourceRectChanged();
}

void QGfxSourceProxy::setInterpolation(const Interpolation i)
{
    if (m_interpolation == i) {
        return;
    }
    m_interpolation = i;
    polish();
    Q_EMIT interpolationChanged();
}


void QGfxSourceProxy::useProxy()
{
    if (!m_proxy) {
        m_proxy.reset(new QQuickShaderEffectSource(this));
    }
    m_proxy->setSourceRect(m_sourceRect);
    m_proxy->setSourceItem(m_input);
    m_proxy->setSmooth(m_interpolation != Interpolation::Nearest);
    setOutput(m_proxy.get());
}

void QGfxSourceProxy::repolish()
{
    polish();
}

QQuickItemLayer *QGfxSourceProxy::findLayer(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return nullptr;
    }
    const QQuickItemPrivate * const d = QQuickItemPrivate::get(item);
    Q_ASSERT(d);
    if (!d) {
        return nullptr;
    }
    QQuickItemLayer *layer = d->layer();
    if (layer && layer->enabled()) {
        return layer;
    }
    return nullptr;
}

void QGfxSourceProxy::updatePolish()
{
    if (m_input == nullptr) {
        setOutput(nullptr);
        return;
    }

    const auto image = qobject_cast<QQuickImage *>(m_input);
    const auto shaderSource = qobject_cast<QQuickShaderEffectSource *>(m_input);
    const bool childless = m_input->childItems().isEmpty();
    const bool interpOk = (m_interpolation == Interpolation::Any)
                    || ((m_interpolation == Interpolation::Linear) && (m_input->smooth() == true))
                    || ((m_interpolation == Interpolation::Nearest) && (m_input->smooth() == false));

    // Layers can be used in two different ways. Option 1 is when the item is
    // used as input to a separate ShaderEffect component. In this case,
    // m_input will be the item itself.
    QQuickItemLayer *layer = findLayer(m_input);
    if (!layer && shaderSource) {
        // Alternatively, the effect is applied via layer.effect, and the
        // input to the effect will be the layer's internal ShaderEffectSource
        // item. In this case, we need to backtrack and find the item that has
        // the layer and configure it accordingly.
        layer = findLayer(shaderSource->sourceItem());
    }

    // A bit crude test, but we're only using source rect for
    // blurring+transparent edge, so this is good enough.
    const bool padded = ((m_sourceRect.x() < 0) || (m_sourceRect.y() < 0));

    bool direct = false;

    if (layer) {
        // Auto-configure the layer so interpolation and padding works as
        // expected without allocating additional FBOs. In edgecases, where
        // this feature is undesiered, the user can simply use
        // ShaderEffectSource rather than layer.
        layer->setProperty("sourceRect", m_sourceRect);
        layer->setProperty("smooth", (m_interpolation != Interpolation::Nearest));
        direct = true;
    } else if (childless && interpOk) {
        if (shaderSource) {
            if ((shaderSource->sourceRect() == m_sourceRect) || m_sourceRect.isEmpty()) {
                direct = true;
            }
        } else if (!padded
                   && ((image && (image->fillMode() == QQuickImage::Stretch)
                        && !image->sourceSize().isNull())
                       || (!image && m_input->isTextureProvider()))) {
            direct = true;
        }
    }

    if (direct) {
        setOutput(m_input);
    } else {
        useProxy();
    }

    // Remove the proxy if it is not in use..
    if (m_proxy && (m_output == m_input)) {
        m_proxy.reset();
    }
}

QT_END_NAMESPACE

#include "moc_qgfxsourceproxy_p.cpp"
