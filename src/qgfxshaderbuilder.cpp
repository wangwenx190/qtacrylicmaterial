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

#include "qgfxshaderbuilder_p.h"
#include <QtCore/qdebug.h>
#include <QtCore/qmath.h>
#include <QtCore/qnumeric.h>
#include <QtCore/qtemporaryfile.h>
#include <QtCore/qvarlengtharray.h>
#include <QtGui/qoffscreensurface.h>
#include <QtGui/qopenglcontext.h>
#include <QtGui/qopenglfunctions.h>
#include <QtQuick/qquickwindow.h>

#ifndef GL_MAX_VARYING_COMPONENTS
#  define GL_MAX_VARYING_COMPONENTS (0x8B4B)
#endif

#ifndef GL_MAX_VARYING_FLOATS
#  define GL_MAX_VARYING_FLOATS (0x8B4B)
#endif

#ifndef GL_MAX_VARYING_VECTORS
#  define GL_MAX_VARYING_VECTORS (0x8DFC)
#endif

#ifndef QT5COMPAT_MAX_BLUR_SAMPLES
#  define QT5COMPAT_MAX_BLUR_SAMPLES (15) // Conservative estimate for maximum varying vectors in
                                          // shaders (maximum 60 components on some Metal
                                          // implementations, hence 15 vectors of 4 components each)
#elif !defined(QT5COMPAT_MAX_BLUR_SAMPLES_GL)
#  define QT5COMPAT_MAX_BLUR_SAMPLES_GL QT5COMPAT_MAX_BLUR_SAMPLES
#endif

#ifndef QT5COMPAT_MAX_BLUR_SAMPLES_GL
#  define QT5COMPAT_MAX_BLUR_SAMPLES_GL (8) // minimum number of varyings in the ES 2.0 spec.
#endif

QT_BEGIN_NAMESPACE

QGfxShaderBuilder::QGfxShaderBuilder(QObject *parent) : QObject(parent)
{
    QList<QShaderBaker::GeneratedShader> targets = {};

    const QSGRendererInterface::GraphicsApi graphicsApi = QQuickWindow::graphicsApi();
    switch (graphicsApi) {
    case QSGRendererInterface::Direct3D11:
        targets.append({ QShader::HlslShader, QShaderVersion(50) });
        break;
    case QSGRendererInterface::OpenGL:
        targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
        targets.append({ QShader::GlslShader, QShaderVersion(120) });
        targets.append({ QShader::GlslShader, QShaderVersion(150) });
        break;
    case QSGRendererInterface::Metal:
        targets.append({ QShader::MslShader, QShaderVersion(12) });
        break;
    case QSGRendererInterface::Vulkan:
        targets.append({ QShader::SpirvShader, QShaderVersion(100) });
        break;
    default:
        qWarning() << "QGfxShaderBuilder: Unsupported graphics backend. No shaders will be generated.";
        break;
    }

    m_shaderBaker.setGeneratedShaders(targets);
    m_shaderBaker.setGeneratedShaderVariants({ QShader::StandardShader,
                                               QShader::BatchableVertexShader });

#if QT_CONFIG(opengl)
    if (graphicsApi == QSGRendererInterface::OpenGL) {
        // The following code makes the assumption that an OpenGL context the GUI
        // thread will get the same capabilities as the render thread's OpenGL
        // context. Not 100% accurate, but it works...
        QOpenGLContext context{};
        if (!context.create()) {
            qDebug() << "Failed to acquire GL context to resolve capabilities, using defaults..";
            m_maxBlurSamples = QT5COMPAT_MAX_BLUR_SAMPLES_GL;
            return;
        }

        QOffscreenSurface surface{};
        // In very odd cases, we can get incompatible configs here unless we pass the
        // GL context's format on to the offscreen format.
        surface.setFormat(context.format());
        surface.create();

        QOpenGLContext *oldContext = QOpenGLContext::currentContext();
        QSurface *oldSurface = (oldContext ? oldContext->surface() : nullptr);
        if (context.makeCurrent(&surface)) {
            QOpenGLFunctions *gl = context.functions();
            if (context.isOpenGLES()) {
                gl->glGetIntegerv(GL_MAX_VARYING_VECTORS, &m_maxBlurSamples);
            } else if (context.format().majorVersion() >= 3) {
                int components = 0;
                gl->glGetIntegerv(GL_MAX_VARYING_COMPONENTS, &components);
                m_maxBlurSamples = int(qRound(qreal(components) / 2.0));
            } else {
                int floats = 0;
                gl->glGetIntegerv(GL_MAX_VARYING_FLOATS, &floats);
                m_maxBlurSamples = int(qRound(qreal(floats) / 2.0));
            }
            if (oldContext && oldSurface) {
                oldContext->makeCurrent(oldSurface);
            } else {
                context.doneCurrent();
            }
        } else {
            qDebug() << "QGfxShaderBuilder: Failed to acquire GL context to resolve capabilities, using defaults.";
            m_maxBlurSamples = QT5COMPAT_MAX_BLUR_SAMPLES_GL;
        }
    } else
#endif
    m_maxBlurSamples = QT5COMPAT_MAX_BLUR_SAMPLES;
}

QGfxShaderBuilder::~QGfxShaderBuilder() = default;

/*

    The algorithm works like this..

    For every two pixels we want to sample we take one sample between those
    two pixels and rely on linear interpoliation to get both values at the
    cost of one texture sample. The sample point is calculated based on the
    gaussian weights at the two texels.

    I've included the table here for future reference:

    Requested     Effective       Actual    Actual
    Samples       Radius/Kernel   Samples   Radius(*)
    -------------------------------------------------
    0             0 / 1x1         1         0
    1             0 / 1x1         1         0
    2             1 / 3x3         2         0
    3             1 / 3x3         2         0
    4             2 / 5x5         3         1
    5             2 / 5x5         3         1
    6             3 / 7x7         4         1
    7             3 / 7x7         4         1
    8             4 / 9x9         5         2
    9             4 / 9x9         5         2
    10            5 / 11x11       6         2
    11            5 / 11x11       6         2
    12            6 / 13x13       7         3
    13            6 / 13x13       7         3
    ...           ...             ...       ...

    When ActualSamples is an 'odd' nunber, sample center pixel separately:
    EffectiveRadius: 4
    EffectiveKernel: 9x9
    ActualSamples: 5
     -4  -3  -2  -1   0  +1  +2  +3  +4
    |   |   |   |   |   |   |   |   |   |
      \   /   \   /   |   \   /   \   /
       tL2     tL1    tC   tR1     tR2

    When ActualSamples is an 'even' number, sample 3 center pixels with two
    samples:
    EffectiveRadius: 3
    EffectiveKernel: 7x7
    ActualSamples: 4
     -3  -2  -1   0  +1  +2  +3
    |   |   |   |   |   |   |   |
      \   /   \   /   |    \   /
       tL1     tL0   tR0    tR2

    From this table we have the following formulas:
    EffectiveRadius = RequestedSamples / 2;
    EffectiveKernel = EffectiveRadius * 2 + 1
    ActualSamples   = 1 + RequstedSamples / 2;
    ActualRadius    = RequestedSamples / 4;

    (*) ActualRadius excludes the pixel pair sampled in the center
        for even 'actual sample' counts
*/

[[nodiscard]] static inline qreal qgfx_gaussian(const qreal x, const qreal d)
{
    return qExp((-x * x) / (2.0 * d * d));
}

struct QGfxGaussSample
{
    QByteArray name = {};
    qreal pos = 0.0;
    qreal weight = 0.0;

    inline void set(const QByteArray &n, const qreal p, const qreal w) {
        name = n;
        pos = p;
        weight = w;
    }
};

static inline void qgfx_declareBlur(QByteArray &shader, const QByteArray &direction, QGfxGaussSample *s, const int samples)
{
    for (int i = 0; i != samples; ++i) {
        shader += "layout(location = "_qba + QByteArray::number(i) + ") "_qba + direction + " vec2 "_qba;
        shader += s[i].name;
        shader += ";\n"_qba;
    }
}

static inline void qgfx_buildGaussSamplePoints(QGfxGaussSample *p, const int samples, const int radius, const qreal deviation)
{
    if ((samples % 2) == 1) {
        p[radius].set("tC"_qba, 0.0, 1.0);
        for (int i = 0; i != radius; ++i) {
            const qreal p0 = (((qreal(i) + 1.0) * 2.0) - 1.0);
            const qreal p1 = ((qreal(i) + 1.0) * 2.0);
            const qreal w0 = qgfx_gaussian(p0, deviation);
            const qreal w1 = qgfx_gaussian(p1, deviation);
            qreal w = (w0 + w1);
            qreal samplePos = (((p0 * w0) + (p1 * w1)) / w);
            if (qIsNaN(samplePos)) {
                samplePos = 0.0;
                w = 0.0;
            }
            p[radius - i - 1].set("tL"_qba + QByteArray::number(i), samplePos, w);
            p[radius + i + 1].set("tR"_qba + QByteArray::number(i), -samplePos, w);
        }
    } else {
        { // tL0
            const qreal wl = qgfx_gaussian(-1.0, deviation);
            const qreal wc = qgfx_gaussian(0.0, deviation);
            const qreal w = (wl + wc);
            p[radius].set("tL0"_qba, ((-1.0 * wl) / w), w);
            p[radius + 1].set("tR0"_qba, 1.0, wl); // reuse wl as gauss(-1)==gauss(1);
        }
        for (int i = 0; i != radius; ++i) {
            const qreal p0 = ((qreal(i) + 1.0) * 2.0);
            const qreal p1 = (((qreal(i) + 1.0) * 2.0) + 1.0);
            const qreal w0 = qgfx_gaussian(p0, deviation);
            const qreal w1 = qgfx_gaussian(p1, deviation);
            qreal w = (w0 + w1);
            qreal samplePos = (((p0 * w0) + (p1 * w1)) / w);
            if (qIsNaN(samplePos)) {
                samplePos = 0.0;
                w = 0.0;
            }
            p[radius - i - 1].set("tL"_qba + QByteArray::number(i + 1), samplePos, w);
            p[radius + i + 2].set("tR"_qba + QByteArray::number(i + 1), -samplePos, w);
        }
    }
}

static inline void qgfx_declareUniforms(QByteArray &shader, const bool alphaOnly)
{
    shader += "layout(std140, binding = 0) uniform buf {\n"
              "    mat4 qt_Matrix;\n"
              "    float qt_Opacity;\n"
              "    float spread;\n"
              "    vec2 dirstep;\n"_qba;
    if (alphaOnly) {
        shader += "    vec4 color;\n"
                  "    float thickness;\n"_qba;
    }
    shader += "};\n\n"_qba;
}

[[nodiscard]] static inline QByteArray qgfx_gaussianVertexShader(QGfxGaussSample *p, const int samples, const bool alphaOnly)
{
    QByteArray shader = {};
    shader.reserve(1024);
    shader += "#version 440\n\n"
              "layout(location = 0) in vec4 qt_Vertex;\n"
              "layout(location = 1) in vec2 qt_MultiTexCoord0;\n\n"_qba;

    qgfx_declareUniforms(shader, alphaOnly);

    shader += "out gl_PerVertex { vec4 gl_Position; };\n\n"_qba;

    qgfx_declareBlur(shader, "out"_qba, p, samples);

    shader += "\nvoid main() {\n"
              "    gl_Position = qt_Matrix * qt_Vertex;\n\n"_qba;

    for (int i = 0; i != samples; ++i) {
        shader += "    "_qba;
        shader += p[i].name;
        shader += " = qt_MultiTexCoord0"_qba;
        if (!qFuzzyCompare(p[i].pos, 0.0)) {
            shader += " + spread * dirstep * float("_qba;
            shader += QByteArray::number(p[i].pos);
            shader += ")"_qba;
        }
        shader += ";\n"_qba;
    }

    shader += "}\n"_qba;

    return shader;
}

[[nodiscard]] static inline QByteArray qgfx_gaussianFragmentShader(QGfxGaussSample *p, const int samples, const bool alphaOnly)
{
    QByteArray shader = {};
    shader.reserve(1024);
    shader += "#version 440\n\n"_qba;

    qgfx_declareUniforms(shader, alphaOnly);

    shader += "layout(binding = 1) uniform sampler2D source;\n"_qba;
    shader += "layout(location = 0) out vec4 fragColor;\n"_qba;

    qgfx_declareBlur(shader, "in"_qba, p, samples);

    shader += "\nvoid main() {\n"
              "    fragColor = "_qba;
    if (alphaOnly) {
        shader += "mix(vec4(0), color, clamp(("_qba;
    } else {
        shader += "("_qba;
    }

    qreal sum = 0.0;
    for (int i = 0; i != samples; ++i) {
        sum += p[i].weight;
    }

    for (int i = 0; i != samples; ++i) {
        shader += "\n                    + float("_qba;
        shader += QByteArray::number(p[i].weight / sum);
        shader += ") * texture(source, "_qba;
        shader += p[i].name;
        shader += ")"_qba;
        if (alphaOnly) {
            shader += ".a"_qba;
        }
    }

    shader += "\n                   )"_qba;
    if (alphaOnly) {
        shader += "/thickness, 0.0, 1.0))"_qba;
    }
    shader += "* qt_Opacity;\n}"_qba;

    return shader;
}

[[nodiscard]] static inline QByteArray qgfx_fallbackVertexShader(const bool alphaOnly)
{
    QByteArray vertexShader =
           "#version 440\n\n"
           "layout(location = 0) in vec4 qt_Vertex;\n"
           "layout(location = 1) in vec2 qt_MultiTexCoord0;\n\n"_qba;

    qgfx_declareUniforms(vertexShader, alphaOnly);

    vertexShader +=
           "layout(location = 0) out vec2 qt_TexCoord0;\n"
           "out gl_PerVertex { vec4 gl_Position; };\n"
           "void main() {\n"
           "    gl_Position = qt_Matrix * qt_Vertex;\n"
           "    qt_TexCoord0 = qt_MultiTexCoord0;\n"
           "}\n"_qba;

    return vertexShader;
}

[[nodiscard]] static inline QByteArray qgfx_fallbackFragmentShader(const int requestedRadius, const qreal deviation, const bool masked, const bool alphaOnly)
{
    QByteArray fragShader = "#version 440\n\n"_qba;

    qgfx_declareUniforms(fragShader, alphaOnly);

    fragShader += "layout(binding = 1) uniform sampler2D source;\n"_qba;
    if (masked) {
        fragShader += "layout(binding = 2) uniform sampler2D mask;\n"_qba;
    }

    fragShader +=
        "layout(location = 0) out vec4 fragColor;\n"
        "layout(location = 0) in vec2 qt_TexCoord0;\n"
        "\n"
        "void main() {\n"_qba;
    if (alphaOnly) {
        fragShader += "    float result = 0.0;\n"_qba;
    } else {
        fragShader += "    vec4 result = vec4(0);\n"_qba;
    }
    fragShader += "    vec2 pixelStep = dirstep * spread;\n"_qba;
    if (masked) {
        fragShader += "    pixelStep *= texture(mask, qt_TexCoord0).a;\n"_qba;
    }

    float wSum = 0.0;
    for (int r = -requestedRadius; r <= requestedRadius; ++r) {
        float w = qgfx_gaussian(r, deviation);
        wSum += w;
        fragShader += "    result += float("_qba;
        fragShader += QByteArray::number(w);
        fragShader += ") * texture(source, qt_TexCoord0 + pixelStep * float("_qba;
        fragShader += QByteArray::number(r);
        fragShader += "))"_qba;
        if (alphaOnly) {
            fragShader += ".a"_qba;
        }
        fragShader += ";\n"_qba;
    }
    fragShader += "    const float wSum = float("_qba;
    fragShader += QByteArray::number(wSum);
    fragShader += ");\n"
                  "    fragColor = "_qba;
    if (alphaOnly) {
        fragShader += "mix(vec4(0), color, clamp((result / wSum) / thickness, 0.0, 1.0)) * qt_Opacity;\n"_qba;
    } else {
        fragShader += "(qt_Opacity / wSum) * result;\n"_qba;
    }
    fragShader += "}\n"_qba;

    return fragShader;
}

QVariantMap QGfxShaderBuilder::gaussianBlur(const QJSValue &parameters)
{
    const qreal requestedRadius = qMax(0.0, parameters.property(u"radius"_qs).toNumber());
    const qreal deviation = parameters.property(u"deviation"_qs).toNumber();
    const bool masked = parameters.property(u"masked"_qs).toBool();
    const bool alphaOnly = parameters.property(u"alphaOnly"_qs).toBool();

    const qreal requestedSamples = ((requestedRadius * 2.0) + 1.0);
    const auto samples = int(qRound(1.0 + (requestedSamples / 2.0)));
    const auto radius = int(qRound(requestedSamples / 4.0));
    const bool fallback = parameters.property(u"fallback"_qs).toBool();

    QVariantMap result = {};

    QByteArray vertexShader = {};
    QByteArray fragmentShader = {};
    if (/*(samples > m_maxBlurSamples)*/false || masked || fallback) {
        fragmentShader = qgfx_fallbackFragmentShader(int(qRound(requestedRadius)), deviation, masked, alphaOnly);
        vertexShader = qgfx_fallbackVertexShader(alphaOnly);
    } else {
        QVarLengthArray<QGfxGaussSample, 64> p(samples);
        qgfx_buildGaussSamplePoints(p.data(), samples, radius, deviation);

        fragmentShader = qgfx_gaussianFragmentShader(p.data(), samples, alphaOnly);
        vertexShader = qgfx_gaussianVertexShader(p.data(), samples, alphaOnly);
    }

    result[u"fragmentShader"_qs] = buildFragmentShader(fragmentShader);
    result[u"vertexShader"_qs] = buildVertexShader(vertexShader);
    return result;
}

QUrl QGfxShaderBuilder::buildFragmentShader(const QByteArray &code)
{
    m_fragmentShader.reset(new QTemporaryFile(this));
    m_fragmentShader->setAutoRemove(false); // We need a permanent file, so disable automatic deletion.

    return buildShader(code, QShader::FragmentStage, m_fragmentShader.get());
}

QUrl QGfxShaderBuilder::buildVertexShader(const QByteArray &code)
{
    m_vertexShader.reset(new QTemporaryFile(this));
    m_vertexShader->setAutoRemove(false); // We need a permanent file, so disable automatic deletion.

    return buildShader(code, QShader::VertexStage, m_vertexShader.get());
}

QUrl QGfxShaderBuilder::buildShader(const QByteArray &code,
                                    const QShader::Stage stage,
                                    QTemporaryFile *output)
{
    if (!output->open()) {
        qWarning() << "QGfxShaderBuilder: Failed to create temporary files";
        return {};
    }

    m_shaderBaker.setSourceString(code, stage, output->fileName());
    const QShader compiledShader = m_shaderBaker.bake();
    if (!compiledShader.isValid()) {
        output->close();
        qWarning() << "QGfxShaderBuilder: Failed to compile shader for stage "
                   << stage << ": "
                   << m_shaderBaker.errorMessage()
                   << QString::fromUtf8(code).replace(QChar(u'\n'), QChar(QChar::LineFeed));
        return {};
    }

    output->write(compiledShader.serialized());
    output->close();

    return QUrl::fromLocalFile(output->fileName());
}

QT_END_NAMESPACE

#include "moc_qgfxshaderbuilder_p.cpp"
