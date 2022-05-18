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

#include "qtacrylicmaterial_global.h"
#include <QtCore/qobject.h>
#include <QtCore/qmap.h>
#include <QtCore/qurl.h>
#include <QtShaderTools/private/qshaderbaker_p.h>
#include <QtQml/qqml.h>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE

class QJSValue;
class QTemporaryFile;

class QTACRYLICMATERIAL_API QGfxShaderBuilder : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ShaderBuilder)
    Q_DISABLE_COPY_MOVE(QGfxShaderBuilder)

public:
    explicit QGfxShaderBuilder(QObject *parent = nullptr);
    ~QGfxShaderBuilder() override;

public Q_SLOTS:
    [[nodiscard]] QVariantMap gaussianBlur(const QJSValue &parameters);
    [[nodiscard]] QUrl buildVertexShader(const QByteArray &code);
    [[nodiscard]] QUrl buildFragmentShader(const QByteArray &code);

private:
    [[nodiscard]] QUrl buildShader(const QByteArray &code, const QShader::Stage stage, QTemporaryFile *output);

private:
    int m_maxBlurSamples = 0;
    QShaderBaker m_shaderBaker = {};

    QScopedPointer<QTemporaryFile> m_fragmentShader;
    QScopedPointer<QTemporaryFile> m_vertexShader;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QT_PREPEND_NAMESPACE(QGfxShaderBuilder))
