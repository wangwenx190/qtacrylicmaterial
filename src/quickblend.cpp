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

#include "quickblend.h"
#include "quickblend_p.h"
#include "qgfxsourceproxy_p.h"
#include "qgfxshaderbuilder_p.h"
#include <QtQuick/private/qquickshadereffect_p.h>
#include <QtQuick/private/qquickshadereffectsource_p.h>
#include <QtQuick/private/qquickanchors_p.h>

static const QByteArray fragmentShaderBegin = R"(#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
};
layout(binding = 1) uniform sampler2D source;
layout(binding = 2) uniform sampler2D foregroundSource;

float RGBtoL(vec3 color) {
    float cmin = min(color.r, min(color.g, color.b));
    float cmax = max(color.r, max(color.g, color.b));
    float l = (cmin + cmax) / 2.0;
    return l;
}

vec3 RGBtoHSL(vec3 color) {
    float cmin = min(color.r, min(color.g, color.b));
    float cmax = max(color.r, max(color.g, color.b));
    float h = 0.0;
    float s = 0.0;
    float l = (cmin + cmax) / 2.0;
    float diff = cmax - cmin;

    if (diff > 1.0 / 256.0) {
        if (l < 0.5)
            s = diff / (cmin + cmax);
        else
            s = diff / (2.0 - (cmin + cmax));

        if (color.r == cmax)
            h = (color.g - color.b) / diff;
        else if (color.g == cmax)
            h = 2.0 + (color.b - color.r) / diff;
        else
            h = 4.0 + (color.r - color.g) / diff;

        h /= 6.0;
    }
    return vec3(h, s, l);
}

float hueToIntensity(float v1, float v2, float h) {
    h = fract(h);
    if (h < 1.0 / 6.0)
        return v1 + (v2 - v1) * 6.0 * h;
    else if (h < 1.0 / 2.0)
        return v2;
    else if (h < 2.0 / 3.0)
        return v1 + (v2 - v1) * 6.0 * (2.0 / 3.0 - h);

    return v1;
}

vec3 HSLtoRGB(vec3 color) {
    float h = color.x;
    float l = color.z;
    float s = color.y;

    if (s < 1.0 / 256.0)
        return vec3(l, l, l);

    float v1;
    float v2;
    if (l < 0.5)
        v2 = l * (1.0 + s);
    else
        v2 = (l + s) - (s * l);

    v1 = 2.0 * l - v2;

    float d = 1.0 / 3.0;
    float r = hueToIntensity(v1, v2, h + d);
    float g = hueToIntensity(v1, v2, h);
    float b = hueToIntensity(v1, v2, h - d);
    return vec3(r, g, b);
}

float channelBlendHardLight(float c1, float c2) {
    return c2 > 0.5 ? (1.0 - (1.0 - 2.0 * (c2 - 0.5)) * (1.0 - c1)) : (2.0 * c1 * c2);
}

void main() {
    vec4 result = vec4(0.0);
    vec4 color1 = texture(source, qt_TexCoord0);
    vec4 color2 = texture(foregroundSource, qt_TexCoord0);
    vec3 rgb1 = color1.rgb / max(1.0/256.0, color1.a);
    vec3 rgb2 = color2.rgb / max(1.0/256.0, color2.a);
    float a = max(color1.a, color1.a * color2.a);
)"_qba;

static const QByteArray fragmentShaderEnd = R"(
    fragColor.rgb = mix(rgb1, result.rgb, color2.a);
    fragColor.rbg *= a;
    fragColor.a = a;
    fragColor *= qt_Opacity;
}
)"_qba;

static const QByteArray blendModeAddition = "result.rgb = min(rgb1 + rgb2, 1.0);\n"_qba;
static const QByteArray blendModeAverage = "result.rgb = 0.5 * (rgb1 + rgb2);\n"_qba;
static const QByteArray blendModeColor = "result.rgb = HSLtoRGB(vec3(RGBtoHSL(rgb2).xy, RGBtoL(rgb1)));\n"_qba;
static const QByteArray blendModeColorBurn = "result.rgb = clamp(1.0 - ((1.0 - rgb1) / max(vec3(1.0 / 256.0), rgb2)), vec3(0.0), vec3(1.0));\n"_qba;
static const QByteArray blendModeColorDodge = "result.rgb = clamp(rgb1 / max(vec3(1.0 / 256.0), (1.0 - rgb2)), vec3(0.0), vec3(1.0));\n"_qba;
static const QByteArray blendModeDarken = "result.rgb = min(rgb1, rgb2);\n"_qba;
static const QByteArray blendModeDarkerColor = "result.rgb = 0.3 * rgb1.r + 0.59 * rgb1.g + 0.11 * rgb1.b > 0.3 * rgb2.r + 0.59 * rgb2.g + 0.11 * rgb2.b ? rgb2 : rgb1;\n"_qba;
static const QByteArray blendModeDifference = "result.rgb = abs(rgb1 - rgb2);\n"_qba;
static const QByteArray blendModeDivide = "result.rgb = clamp(rgb1 / rgb2, 0.0, 1.0);\n"_qba;
static const QByteArray blendModeExclusion = "result.rgb = rgb1 + rgb2 - 2.0 * rgb1 * rgb2;\n"_qba;
static const QByteArray blendModeHardLight = "result.rgb = vec3(channelBlendHardLight(rgb1.r, rgb2.r), channelBlendHardLight(rgb1.g, rgb2.g), channelBlendHardLight(rgb1.b, rgb2.b));\n"_qba;
static const QByteArray blendModeHue = "result.rgb = HSLtoRGB(vec3(RGBtoHSL(rgb2).x, RGBtoHSL(rgb1).yz));\n"_qba;
static const QByteArray blendModeLighten = "result.rgb = max(rgb1, rgb2);\n"_qba;
static const QByteArray blendModeLighterColor = "result.rgb = 0.3 * rgb1.r + 0.59 * rgb1.g + 0.11 * rgb1.b > 0.3 * rgb2.r + 0.59 * rgb2.g + 0.11 * rgb2.b ? rgb1 : rgb2;\n"_qba;
static const QByteArray blendModeLightness = "result.rgb = HSLtoRGB(vec3(RGBtoHSL(rgb1).xy, RGBtoL(rgb2)));\n"_qba;
static const QByteArray blendModeMultiply = "result.rgb = rgb1 * rgb2;\n"_qba;
static const QByteArray blendModeNegation = "result.rgb = 1.0 - abs(1.0 - rgb1 - rgb2);\n"_qba;
static const QByteArray blendModeNormal = "result.rgb = rgb2; a = max(color1.a, color2.a);\n"_qba;
static const QByteArray blendModeSaturation = "vec3 hsl1 = RGBtoHSL(rgb1); result.rgb = HSLtoRGB(vec3(hsl1.x, RGBtoHSL(rgb2).y, hsl1.z));\n"_qba;
static const QByteArray blendModeScreen = "result.rgb = 1.0 - (vec3(1.0) - rgb1) * (vec3(1.0) - rgb2);\n"_qba;
static const QByteArray blendModeSubtract = "result.rgb = max(rgb1 - rgb2, vec3(0.0));\n"_qba;
static const QByteArray blendModeSoftLight = "result.rgb = rgb1 * ((1.0 - rgb1) * rgb2 + (1.0 - (1.0 - rgb1) * (1.0 - rgb2)));\n"_qba;
static const QByteArray blendModeDefault = "gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"_qba;

QuickBlendPrivate::QuickBlendPrivate(QuickBlend *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

QuickBlendPrivate::~QuickBlendPrivate() = default;

QuickBlendPrivate *QuickBlendPrivate::get(QuickBlend *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const QuickBlendPrivate *QuickBlendPrivate::get(const QuickBlend *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

void QuickBlendPrivate::buildFragmentShader()
{
    m_shaderItem->setProperty("source", QVariant::fromValue(m_backgroundSourceProxy->output()));
    m_shaderItem->setProperty("foregroundSource", QVariant::fromValue(m_foregroundSourceProxy->output()));
    const QByteArray shader = generateShaderCode(m_mode);
    const QUrl fragmentShaderUrl = m_shaderBuilder->buildFragmentShader(shader);
    m_shaderItem->setFragmentShader(fragmentShaderUrl);
}

void QuickBlendPrivate::initialize()
{
    Q_Q(QuickBlend);
    connect(q, &QuickBlend::modeChanged, this, &QuickBlendPrivate::buildFragmentShader);

    m_backgroundSourceProxy.reset(new QGfxSourceProxy(q));
    connect(m_backgroundSourceProxy.get(), &QGfxSourceProxy::outputChanged, this, &QuickBlendPrivate::buildFragmentShader);
    m_foregroundSourceProxy.reset(new QGfxSourceProxy(q));
    connect(m_foregroundSourceProxy.get(), &QGfxSourceProxy::outputChanged, this, &QuickBlendPrivate::buildFragmentShader);
    m_shaderBuilder.reset(new QGfxShaderBuilder(q));
    m_shaderItem.reset(new QQuickShaderEffect(q));
    const auto shaderItemAnchors = new QQuickAnchors(m_shaderItem.get(), m_shaderItem.get());
    shaderItemAnchors->setFill(q);
    m_cacheItem.reset(new QQuickShaderEffectSource(q));
    const auto cacheItemAnchors = new QQuickAnchors(m_cacheItem.get(), m_cacheItem.get());
    cacheItemAnchors->setFill(q);
    m_cacheItem->setSmooth(true);
    m_cacheItem->setSourceItem(m_shaderItem.get());
    m_cacheItem->setLive(true);
    m_cacheItem->setHideSource(m_cached);
    m_cacheItem->setVisible(m_cached);

    buildFragmentShader();
}

QByteArray QuickBlendPrivate::generateShaderCode(const Mode mode) const
{
    QByteArray shader = fragmentShaderBegin;
    switch (mode) {
    case Mode::Addition:
        shader += blendModeAddition;
        break;
    case Mode::Average:
        shader += blendModeAverage;
        break;
    case Mode::Color:
        shader += blendModeColor;
        break;
    case Mode::ColorBurn:
        shader += blendModeColorBurn;
        break;
    case Mode::ColorDodge:
        shader += blendModeColorDodge;
        break;
    case Mode::Darken:
        shader += blendModeDarken;
        break;
    case Mode::DarkerColor:
        shader += blendModeDarkerColor;
        break;
    case Mode::Difference:
        shader += blendModeDifference;
        break;
    case Mode::Divide:
        shader += blendModeDivide;
        break;
    case Mode::Exclusion:
        shader += blendModeExclusion;
        break;
    case Mode::HardLight:
        shader += blendModeHardLight;
        break;
    case Mode::Hue:
        shader += blendModeHue;
        break;
    case Mode::Lighten:
        shader += blendModeLighten;
        break;
    case Mode::LighterColor:
        shader += blendModeLighterColor;
        break;
    case Mode::Lightness:
        shader += blendModeLightness;
        break;
    case Mode::Negation:
        shader += blendModeNegation;
        break;
    case Mode::Normal:
        shader += blendModeNormal;
        break;
    case Mode::Multiply:
        shader += blendModeMultiply;
        break;
    case Mode::Saturation:
        shader += blendModeSaturation;
        break;
    case Mode::Screen:
        shader += blendModeScreen;
        break;
    case Mode::Subtract:
        shader += blendModeSubtract;
        break;
    case Mode::SoftLight:
        shader += blendModeSoftLight;
        break;
    }
    shader += fragmentShaderEnd;
    return shader;
}

QuickBlend::QuickBlend(QQuickItem *parent)
    : QQuickItem(parent), d_ptr(new QuickBlendPrivate(this))
{
    qRegisterMetaType<Mode>();
}

QuickBlend::~QuickBlend() = default;

QQuickItem *QuickBlend::background() const
{
    Q_D(const QuickBlend);
    return d->m_background;
}

void QuickBlend::setBackground(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_D(QuickBlend);
    if (d->m_background == item) {
        return;
    }
    d->m_background = item;
    d->m_backgroundSourceProxy->setInput(d->m_background);
    Q_EMIT backgroundChanged();
}

QQuickItem *QuickBlend::foreground() const
{
    Q_D(const QuickBlend);
    return d->m_foreground;
}

void QuickBlend::setForeground(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_D(QuickBlend);
    if (d->m_foreground == item) {
        return;
    }
    d->m_foreground = item;
    d->m_foregroundSourceProxy->setInput(d->m_foreground);
    Q_EMIT foregroundChanged();
}

QuickBlend::Mode QuickBlend::mode() const
{
    Q_D(const QuickBlend);
    return d->m_mode;
}

void QuickBlend::setMode(const Mode value)
{
    Q_D(QuickBlend);
    if (d->m_mode == value) {
        return;
    }
    d->m_mode = value;
    Q_EMIT modeChanged();
}

bool QuickBlend::isCached() const
{
    Q_D(const QuickBlend);
    return d->m_cached;
}

void QuickBlend::setCached(const bool value)
{
    Q_D(QuickBlend);
    if (d->m_cached == value) {
        return;
    }
    d->m_cached = value;
    d->m_cacheItem->setHideSource(d->m_cached);
    d->m_cacheItem->setVisible(d->m_cached);
    Q_EMIT cachedChanged();
}
