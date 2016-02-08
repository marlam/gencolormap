/*
 * Copyright (C) 2015, 2016 Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <algorithm>
#include <vector>
#include <limits>
#include <cmath>
#include <cstring>

#include "colormap.hpp"

/* Notes about the color spaces used internally:
 *
 * - We use D65 white everywhere
 * - RGB means linear RGB; we also have sRGB
 * - RGB and sRGB values are in [0,1]
 * - XYZ, LUV, and similar values are in the original range (not normalized);
 *   often this is [0,100]
 * - All angles (for hue) are measured in radians
 */

namespace ColorMap {

/* Generic helpers */

static const float pi = M_PI;
static const float twopi = 2.0 * M_PI;

static float clamp(float x, float lo, float hi)
{
    return std::min(std::max(x, lo), hi);
}

static float hue_diff(float h0, float h1)
{
    float t = std::fabs(h1 - h0);
    return (t < pi ? t : twopi - t);
}

static float uchar_to_float(unsigned char x)
{
    return x / 255.0f;
}

static unsigned char float_to_uchar(float x)
{
    return std::round(x * 255.0f);
}

/* XYZ and related color spaces helper functions and values */

static float u_prime(float x, float y, float z)
{
    return 4.0f * x / (x + 15.0f * y + 3.0f * z);
}

static float v_prime(float x, float y, float z)
{
    return 9.0f * y / (x + 15.0f * y + 3.0f * z);
}

static const float d65_x =  95.047f;
static const float d65_y = 100.000f;
static const float d65_z = 108.883f;
static const float d65_u_prime = u_prime(d65_x, d65_y, d65_z);
static const float d65_v_prime = v_prime(d65_x, d65_y, d65_z);

/* Color space conversion: LCH <-> LUV */

static float lch_saturation(float l, float c)
{
    return c / std::max(l, 1e-8f);
}

static float lch_chroma(float l, float s)
{
    return s * l;
}

static void lch_to_luv(float c, float h, float* u, float* v)
{
    *u = c * std::cos(h);
    *v = c * std::sin(h);
}

static void luv_to_lch(float u, float v, float* c, float* h)
{
    *c = std::hypot(u, v);
    *h = std::atan2(v, u);
    if (*h < 0.0f)
        *h += twopi;
}

static float luv_saturation(float l, float u, float v)
{
    return lch_saturation(l, std::hypot(u, v));
}

/* Color space conversion: LUV <-> XYZ */

static void luv_to_xyz(float l, float u, float v, float* x, float* y, float* z)
{
    float u_prime = u / (13.0f * l) + d65_u_prime;
    float v_prime = v / (13.0f * l) + d65_v_prime;
    if (l <= 8.0f) {
        *y = d65_y * l * (3.0f * 3.0f * 3.0f / (29.0f * 29.0f * 29.0f));
    } else {
        float tmp = (l + 16.0f) / 116.0f;
        *y = d65_y * tmp * tmp * tmp;
    }
    *x = (*y) * (9.0f * u_prime) / (4.0f * v_prime);
    *z = (*y) * (12.0f - 3.0f * u_prime - 20.0f * v_prime) / (4.0f * v_prime);
}

static void xyz_to_luv(float x, float y, float z, float* l, float* u, float* v)
{
    float y_ratio = y / d65_y;
    if (y_ratio <= (6.0f * 6.0f * 6.0f) / (29.0f * 29.0f * 29.0f)) {
        *l = (29.0f * 29.0f * 29.0f) / (3.0f * 3.0f * 3.0f) * y_ratio;
    } else {
        *l = 116.0f * std::cbrt(y_ratio) - 16.0f;
    }
    *u = 13.0f * (*l) * (u_prime(x, y, z) - d65_u_prime);
    *v = 13.0f * (*l) * (v_prime(x, y, z) - d65_v_prime);
}

/* Color space conversion: LAB <-> XYZ */

static float lab_invf(float t)
{
    if (t > 6.0f / 29.0f)
        return t * t * t;
    else
        return (3.0f * 6.0f * 6.0f) / (29.0f * 29.0f) * (t - 4.0f / 29.0f);
}

static void lab_to_xyz(float l, float a, float b, float* x, float* y, float* z)
{
    float t = (l + 16.0f) / 116.0f;
    *x = d65_x * lab_invf(t + a / 500.0f);
    *y = d65_y * lab_invf(t);
    *z = d65_z * lab_invf(t - b / 200.0f);
}

static float lab_f(float t)
{
    if (t > (6.0f * 6.0f * 6.0f) / (29.0f * 29.0f * 29.0f))
        return std::cbrt(t);
    else
        return (29.0f * 29.0f) / (3.0f * 6.0f * 6.0f) * t + 4.0f / 29.0f;
}

static void xyz_to_lab(float x, float y, float z, float* l, float* a, float* b)
{
    float fx = lab_f(x / d65_x);
    float fy = lab_f(y / d65_y);
    float fz = lab_f(z / d65_z);
    *l = 116.0f * fy - 16.0f;
    *a = 500.0f * (fx - fy);
    *b = 200.0f * (fy - fz);
}

/* Color space conversion: RGB <-> XYZ */

static void rgb_to_xyz(float r, float g, float b, float* x, float* y, float *z)
{
    *x = (0.4124f * r + 0.3576f * g + 0.1805f * b) * 100.0f;
    *y = (0.2126f * r + 0.7152f * g + 0.0722f * b) * 100.0f;
    *z = (0.0193f * r + 0.1192f * g + 0.9505f * b) * 100.0f;
}

static void xyz_to_rgb(float x, float y, float z, float* r, float* g, float* b)
{
    *r = clamp((+3.2406255f * x - 1.5372080f * y - 0.4986286f * z) / 100.0f, 0.0f, 1.0f);
    *g = clamp((-0.9689307f * x + 1.8757561f * y + 0.0415175f * z) / 100.0f, 0.0f, 1.0f);
    *b = clamp((+0.0557101f * x - 0.2040211f * y + 1.0569959f * z) / 100.0f, 0.0f, 1.0f);
}

/* Color space conversion: RGB <-> sRGB */

static float rgb_to_srgb_helper(float x)
{
    return (x <= 0.0031308f ? (x * 12.92f) : (1.055f * std::pow(x, 1.0f / 2.4f) - 0.055f));
}

static void rgb_to_srgb(float r, float g, float b, float* sr, float* sg, float* sb)
{
    *sr = rgb_to_srgb_helper(r);
    *sg = rgb_to_srgb_helper(g);
    *sb = rgb_to_srgb_helper(b);
}

static float srgb_to_rgb_helper(float x)
{
    return (x <= 0.04045f ? (x / 12.92f) : std::pow((x + 0.055f) / 1.055f, 2.4f));
}

static void srgb_to_rgb(float sr, float sg, float sb, float* r, float* g, float* b)
{
    *r = srgb_to_rgb_helper(sr);
    *g = srgb_to_rgb_helper(sg);
    *b = srgb_to_rgb_helper(sb);
}

/* Helpers for LUV colors */

typedef struct {
    float l;
    float u;
    float v;
} LUVColor;

LUVColor operator+(LUVColor a, LUVColor b)
{
    LUVColor c = { .l = a.l + b.l, .u = a.u + b.u, .v = a.v + b.v };
    return c;
}

LUVColor operator*(float a, LUVColor b)
{
    LUVColor c = { .l = a * b.l, .u = a * b.u, .v = a * b.v };
    return c;
}

static float srgb_to_lch_hue(float sr, float sg, float sb)
{
    float r, g, b;
    srgb_to_rgb(sr, sg, sb, &r, &g, &b);
    float x, y, z;
    rgb_to_xyz(r, g, b, &x, &y, &z);
    float l, u, v;
    xyz_to_luv(x, y, z, &l, &u, &v);
    float c, h;
    luv_to_lch(u, v, &c, &h);
    return h;
}

// Compute most saturated color that fits into the sRGB
// cube for the given LCH hue value. This is the core
// of the paper.
static LUVColor most_saturated_in_srgb(float hue)
{
    /* Static values, only computed once */
    static float h[] = {
        srgb_to_lch_hue(1, 0, 0),
        srgb_to_lch_hue(1, 1, 0),
        srgb_to_lch_hue(0, 1, 0),
        srgb_to_lch_hue(0, 1, 1),
        srgb_to_lch_hue(0, 0, 1),
        srgb_to_lch_hue(1, 0, 1)
    };

    /* RGB values and variable pointers to them */
    int i, j, k;
    if (hue < h[0]) {
        i = 2;
        j = 1;
        k = 0;
    } else if (hue < h[1]) {
        i = 1;
        j = 2;
        k = 0;
    } else if (hue < h[2]) {
        i = 0;
        j = 2;
        k = 1;
    } else if (hue < h[3]) {
        i = 2;
        j = 0;
        k = 1;
    } else if (hue < h[4]) {
        i = 1;
        j = 0;
        k = 2;
    } else if (hue < h[5]) {
        i = 0;
        j = 1;
        k = 2;
    } else {
        i = 2;
        j = 1;
        k = 0;
    }

    /* Compute the missing component */
    float srgb[3];
    float M[3][3] = {
        { 0.4124f, 0.3576f, 0.1805f },
        { 0.2126f, 0.7152f, 0.0722f },
        { 0.0193f, 0.1192f, 0.9505f }
    };
    float alpha = -std::sin(hue);
    float beta = std::cos(hue);
    float T = alpha * d65_u_prime + beta * d65_v_prime;
    srgb[j] = 0.0f;
    srgb[k] = 1.0f;
    float q0 = T * (M[0][k] + 15.0f * M[1][k] + 3.0f * M[2][k]) - (4.0f * alpha * M[0][k] + 9.0f * beta * M[1][k]);
    float q1 = T * (M[0][i] + 15.0f * M[1][i] + 3.0f * M[2][i]) - (4.0f * alpha * M[0][i] + 9.0f * beta * M[1][i]);
    srgb[i] = rgb_to_srgb_helper(clamp(- q0 / q1, 0.0f, 1.0f));

    /* Convert back to LUV */
    float r, g, b;
    srgb_to_rgb(srgb[0], srgb[1], srgb[2], &r, &g, &b);
    float x, y, z;
    rgb_to_xyz(r, g, b, &x, &y, &z);
    float l, u, v;
    xyz_to_luv(x, y, z, &l, &u, &v);
    LUVColor color = { .l = l, .u = u, .v = v };
    return color;
}

static float Smax(float l, float h)
{
    LUVColor pmid = most_saturated_in_srgb(h);
    LUVColor pend = { .l = 0.0f, .u = 0.0f, .v = 0.0f };
    if (l > pmid.l)
        pend.l = 100.0f;
    float alpha = (pend.l - l) / (pend.l - pmid.l);
    float pmids = luv_saturation(pmid.l, pmid.u, pmid.v);
    float pends = luv_saturation(pend.l, pend.u, pend.v);
    return alpha * (pmids - pends) + pends;
}

static LUVColor get_bright_point()
{
    static LUVColor pb = { .l = -1.0f, .u = -1.0f, .v = -1.0f };
    if (pb.l < 0.0f) {
        float x, y, z;
        rgb_to_xyz(1, 1, 0, &x, &y, &z);
        float l, u, v;
        xyz_to_luv(x, y, z, &l, &u, &v);
        pb.l = l;
        pb.u = u;
        pb.v = v;
    }
    return pb;
}

static float mix_hue(float alpha, float h0, float h1)
{
    float M = std::fmod(pi + h1 - h0, twopi) - pi;
    return std::fmod(h0 + alpha * M, twopi);
}

static void get_color_points(float hue, float saturation, float warmth,
        LUVColor pb, float pb_hue, float pb_saturation,
        LUVColor* p0, LUVColor* p1, LUVColor* p2,
        LUVColor* q0, LUVColor* q1, LUVColor* q2)
{
    p0->l = 0.0f;
    lch_to_luv(0.0f, hue, &(p0->u), &(p0->v));
    *p1 = most_saturated_in_srgb(hue);
    float p2l = (1.0f - warmth) * 100.0f + warmth * pb.l;
    float p2h = mix_hue(warmth, hue, pb_hue);
    float p2c = lch_chroma(p2l, std::min(Smax(p2l, p2h), warmth * saturation * pb_saturation));
    p2->l = p2l;
    lch_to_luv(p2c, p2h, &(p2->u), &(p2->v));
    *q0 = (1.0f - saturation) * (*p0) + saturation * (*p1);
    *q2 = (1.0f - saturation) * (*p2) + saturation * (*p1);
    *q1 = 0.5f * ((*q0) + (*q2));
}

static LUVColor B(LUVColor b0, LUVColor b1, LUVColor b2, float t)
{
    float a = (1.0f - t) * (1.0f - t);
    float b = 2.0f * (1.0f - t) * t;
    float c = t * t;
    LUVColor color = a * b0 + b * b1 + c * b2;
    return color;
}

static float invB(float b0, float b1, float b2, float v)
{
    return (b0 - b1 + std::sqrt(std::max(b1 * b1 - b0 * b2 + (b0 - 2.0f * b1 + b2) * v, 0.0f)))
        / (b0 - 2.0f * b1 + b2);
}

static LUVColor get_colormap_entry(float t,
        LUVColor p0, LUVColor p2,
        LUVColor q0, LUVColor q1, LUVColor q2,
        float contrast, float brightness)
{
    float l = 125 - 125 * std::pow(0.2f, (1.0f - contrast) * brightness + t * contrast);
    float T = (l <= q1.l ? 0.5f * invB(p0.l, q0.l, q1.l, l) : 0.5f * invB(q1.l, q2.l, p2.l, l) + 0.5f);
    return (T <= 0.5f ? B(p0, q0, q1, 2.0f * T) : B(q1, q2, p2, 2.0f * (T - 0.5f)));
}

static void convert_colormap_entry(LUVColor color, unsigned char* srgb)
{
    float x, y, z;
    luv_to_xyz(color.l, color.u, color.v, &x, &y, &z);
    float r, g, b;
    xyz_to_rgb(x, y, z, &r, &g, &b);
    float sr, sg, sb;
    rgb_to_srgb(r, g, b, &sr, &sg, &sb);
    srgb[0] = float_to_uchar(sr);
    srgb[1] = float_to_uchar(sg);
    srgb[2] = float_to_uchar(sb);
}

/* Brewer-like color maps */

float BrewerSequentialDefaultContrastForSmallN(int n)
{
    return std::min(0.88f, 0.34f + 0.06f * n);
}

void BrewerSequential(int n, unsigned char* colormap, float hue,
        float contrast, float saturation, float brightness, float warmth)
{
    LUVColor pb, p0, p1, p2, q0, q1, q2;
    pb = get_bright_point();
    float pbc, pbh, pbs;
    luv_to_lch(pb.u, pb.v, &pbc, &pbh);
    pbs = lch_saturation(pb.l, pbc);
    get_color_points(hue, saturation, warmth, pb, pbh, pbs, &p0, &p1, &p2, &q0, &q1, &q2);

    for (int i = 0; i < n; i++) {
        float t = i / (n - 1.0f);
        LUVColor c = get_colormap_entry(t, p0, p2, q0, q1, q2, contrast, brightness);
        convert_colormap_entry(c, colormap + 3 * i);
    }
}

float BrewerDivergingDefaultContrastForSmallN(int n)
{
    return std::min(0.88f, 0.34f + 0.06f * n);
}

void BrewerDiverging(int n, unsigned char* colormap, float hue, float divergence,
        float contrast, float saturation, float brightness, float warmth)
{
    float hue1 = hue + divergence;
    if (hue1 >= twopi)
        hue1 -= twopi;

    LUVColor pb;
    LUVColor p00, p01, p02, q00, q01, q02;
    LUVColor p10, p11, p12, q10, q11, q12;
    pb = get_bright_point();
    float pbc, pbh, pbs;
    luv_to_lch(pb.u, pb.v, &pbc, &pbh);
    pbs = lch_saturation(pb.l, pbc);
    get_color_points(hue,  saturation, warmth, pb, pbh, pbs, &p00, &p01, &p02, &q00, &q01, &q02);
    get_color_points(hue1, saturation, warmth, pb, pbh, pbs, &p10, &p11, &p12, &q10, &q11, &q12);

    for (int i = 0; i < n; i++) {
        LUVColor c;
        if (n % 2 == 1 && i == n / 2) {
            // compute neutral color in the middle of the map
            LUVColor c0 = get_colormap_entry(1.0f, p00, p02, q00, q01, q02, contrast, brightness);
            LUVColor c1 = get_colormap_entry(1.0f, p10, p12, q10, q11, q12, contrast, brightness);
            if (n <= 9) {
                // for discrete color maps, use an extra neutral color
                float c0s = luv_saturation(c0.l, c0.u, c0.v);
                float c1s = luv_saturation(c1.l, c1.u, c1.v);
                float sn = 0.5f * (c0s + c1s) * warmth;
                c.l = 0.5f * (c0.l + c1.l);
                float cc = lch_chroma(c.l, std::min(Smax(c.l, pbh), sn));
                lch_to_luv(cc, pbh, &(c.u), &(c.v));
            } else {
                // for continuous color maps, use an average, since the extra neutral color looks bad
                c = 0.5f * (c0 + c1);
            }
        } else {
            float t = i / (n - 1.0f);
            if (i < n / 2) {
                float tt = 2.0f * t;
                c = get_colormap_entry(tt, p00, p02, q00, q01, q02, contrast, brightness);
            } else {
                float tt = 2.0f * (1.0f - t);
                c = get_colormap_entry(tt, p10, p12, q10, q11, q12, contrast, brightness);
            }
        }
        convert_colormap_entry(c, colormap + 3 * i);
    }
}

void BrewerQualitative(int n, unsigned char* colormap, float hue, float divergence,
        float contrast, float saturation, float brightness)
{
    // Get all information about yellow
    static float yl = -1.0f, yh = -1.0f;
    if (yl < 0.0f) {
        float yx, yy, yz;
        rgb_to_xyz(1, 1, 0, &yx, &yy, &yz);
        float yu, yv;
        xyz_to_luv(yx, yy, yz, &yl, &yu, &yv);
        float yc;
        luv_to_lch(yu, yv, &yc, &yh);
    }

    // Get saturation of red (maximum possible saturation)
    static float rs = -1.0f;
    if (rs < 0.0f) {
        float rx, ry, rz;
        rgb_to_xyz(1, 0, 0, &rx, &ry, &rz);
        float rl, ru, rv;
        xyz_to_luv(rx, ry, rz, &rl, &ru, &rv);
        rs = luv_saturation(rl, ru, rv);
    }

    // Derive parameters of the method
    float eps = hue / twopi;
    float r = divergence / twopi;
    float l0 = brightness * yl;
    float l1 = (1.0f - contrast) * l0;

    // Generate colors
    for (int i = 0; i < n; i++) {
        float t = i / (n - 1.0f);
        float ch = std::fmod(twopi * (eps + t * r), twopi);
        float alpha = hue_diff(ch, yh) / pi;
        float cl = (1.0f - alpha) * l0 + alpha * l1;
        float cs = std::min(Smax(cl, ch), saturation * rs);
        LUVColor c;
        c.l = cl;
        lch_to_luv(lch_chroma(cl, cs), ch, &(c.u), &(c.v));
        convert_colormap_entry(c, colormap + 3 * i);
    }
}

/* Isoluminant */

static void lch_to_colormap(float l, float c, float h,
        int i, unsigned char* colormap)
{
    float u, v;
    lch_to_luv(c, h, &u, &v);
    float x, y, z;
    luv_to_xyz(l, u, v, &x, &y, &z);
    float r, g, b;
    xyz_to_rgb(x, y, z, &r, &g, &b);
    float sr, sg, sb;
    rgb_to_srgb(r, g, b, &sr, &sg, &sb);
    colormap[3 * i + 0] = float_to_uchar(sr);
    colormap[3 * i + 1] = float_to_uchar(sg);
    colormap[3 * i + 2] = float_to_uchar(sb);
}

void IsoluminantSequential(int n, unsigned char* colormap,
        float luminance, float saturation, float hue)
{
    const float l = luminance * 100.0f;
    const float h = hue;
    for (int i = 0; i < n; i++) {
        float t = i / (n - 1.0f);
        float s = saturation * 5.0f * (1.0f - t);
        float c = lch_chroma(l, s);
        lch_to_colormap(l, c, h, i, colormap);
    }
}

void IsoluminantDiverging(int n, unsigned char* colormap,
        float luminance, float saturation, float hue, float divergence)
{
    const float l = luminance * 100.0f;
    for (int i = 0; i < n; i++) {
        float t = i / (n - 1.0f);
        float s = saturation * 5.0f * (t <= 0.5f ? 2.0f * (0.5f - t) : 2.0f * (t - 0.5f));
        float c = lch_chroma(l, s);
        float h = (t <= 0.5f ? hue : hue + divergence);
        lch_to_colormap(l, c, h, i, colormap);
    }
}

void IsoluminantQualitative(int n, unsigned char* colormap,
        float luminance, float saturation, float hue, float divergence)
{
    const float l = luminance * 100.0f;
    const float c = lch_chroma(l, saturation * 5.0f);
    for (int i = 0; i < n; i++) {
        float t = i / (n - 1.0f);
        float h = hue + t * divergence;
        lch_to_colormap(l, c, h, i, colormap);
    }
}

/* CubeHelix */

int CubeHelix(int n, unsigned char* colormap, float hue,
        float rot, float saturation, float gamma)
{
    int clippings = 0;
    for (int i = 0; i < n; i++) {
        float fract = i / (n - 1.0f);
        float angle = twopi * (hue / 3.0f + 1.0f + rot * fract);
        fract = std::pow(fract, gamma);
        float amp = saturation * fract * (1.0f - fract) / 2.0f;
        float s = std::sin(angle);
        float c = std::cos(angle);
        float r = fract + amp * (-0.14861f * c + 1.78277f * s);
        float g = fract + amp * (-0.29227f * c - 0.90649f * s);
        float b = fract + amp * (1.97294f * c);
        bool clipped = false;
        if (r < 0.0f) {
            r = 0.0f;
            clipped = true;
        } else if (r > 1.0f) {
            r = 1.0f;
            clipped = true;
        }
        if (g < 0.0f) {
            g = 0.0f;
            clipped = true;
        } else if (g > 1.0f) {
            g = 1.0f;
            clipped = true;
        }
        if (b < 0.0f) {
            b = 0.0f;
            clipped = true;
        } else if (b > 1.0f) {
            b = 1.0f;
            clipped = true;
        }
        if (clipped)
            clippings++;
        colormap[3 * i + 0] = float_to_uchar(r);
        colormap[3 * i + 1] = float_to_uchar(g);
        colormap[3 * i + 2] = float_to_uchar(b);
    }
    return clippings;
}

/* Moreland */

static void lab_to_msh(float l, float a, float b, float* m, float* s, float* h)
{
    *m = std::sqrt(l * l + a * a + b * b);
    *s = (*m > 0.001f) ? std::acos(l / (*m)) : 0.0f;
    *h = (*s > 0.001f) ? std::atan2(b, a) : 0.0f;
}

static void msh_to_lab(float m, float s, float h, float* l, float* a, float* b)
{
    *l = m * std::cos(s);
    *a = m * std::sin(s) * std::cos(h);
    *b = m * std::sin(s) * std::sin(h);
}

static void srgb_to_msh(unsigned char sr, unsigned char sg, unsigned char sb, float* m, float* s, float *h)
{
    float lr, lg, lb;
    srgb_to_rgb(uchar_to_float(sr), uchar_to_float(sg), uchar_to_float(sb), &lr, &lg, &lb);
    float x, y, z;
    rgb_to_xyz(lr, lg, lb, &x, &y, &z);
    float l, a, b;
    xyz_to_lab(x, y, z, &l, &a, &b);
    lab_to_msh(l, a, b, m, s, h);
}

static void msh_to_srgb(float m, float s, float h, unsigned char* sr, unsigned char* sg, unsigned char* sb)
{
    float l, a, b;
    msh_to_lab(m, s, h, &l, &a, &b);
    float x, y, z;
    lab_to_xyz(l, a, b, &x, &y, &z);
    float lr, lg, lb;
    xyz_to_rgb(x, y, z, &lr, &lg, &lb);
    float fsr, fsg, fsb;
    rgb_to_srgb(lr, lg, lb, &fsr, &fsg, &fsb);
    *sr = float_to_uchar(fsr);
    *sg = float_to_uchar(fsg);
    *sb = float_to_uchar(fsb);
}

static float adjust_hue(float m, float s, float h, float unsaturated_m)
{
    if (m >= unsaturated_m - 0.1f) {
        return h;
    } else {
        float hue_spin = s * std::sqrt(unsaturated_m * unsaturated_m - m * m)
            / (m * std::sin(s));
        if (h > -pi / 3.0f)
            return h + hue_spin;
        else
            return h - hue_spin;
    }
}

void Moreland(int n, unsigned char* colormap,
        unsigned char sr0, unsigned char sg0, unsigned char sb0,
        unsigned char sr1, unsigned char sg1, unsigned char sb1)
{
    float om0, os0, oh0, om1, os1, oh1;
    srgb_to_msh(sr0, sg0, sb0, &om0, &os0, &oh0);
    srgb_to_msh(sr1, sg1, sb1, &om1, &os1, &oh1);
    bool place_white = (os0 >= 0.05f && os1 >= 0.05f && hue_diff(oh0, oh1) > pi / 3.0f);
    float mmid = std::max(std::max(om0, om1), 88.0f);

    for (int i = 0; i < n; i++) {
        float m0 = om0, s0 = os0, h0 = oh0;
        float m1 = om1, s1 = os1, h1 = oh1;
        float t = i / (n - 1.0f);
        if (place_white) {
            if (t < 0.5f) {
                m1 = mmid;
                s1 = 0.0f;
                h1 = 0.0f;
                t *= 2.0f;
            } else {
                m0 = mmid;
                s0 = 0.0f;
                h0 = 0.0f;
                t = 2.0f * t - 1.0f;
            }
        }
        if (s0 < 0.05f && s1 >= 0.05f) {
            h0 = adjust_hue(m1, s1, h1, m0);
        } else if (s0 >= 0.05f && s1 < 0.05f) {
            h1 = adjust_hue(m0, s0, h0, m1);
        }

        float m = (1.0f - t) * m0 + t * m1;
        float s = (1.0f - t) * s0 + t * s1;
        float h = (1.0f - t) * h0 + t * h1;

        msh_to_srgb(m, s, h, colormap + 3 * i + 0, colormap + 3 * i + 1, colormap + 3 * i + 2);
    }
}

/* McNames */

static void cart2pol(float x, float y, float* theta, float* rho)
{
    *theta = std::atan2(y, x);
    *rho = std::hypot(x, y);
}

static void pol2cart(float theta, float rho, float* x, float* y)
{
    *x = rho * std::cos(theta);
    *y = rho * std::sin(theta);
}

static float windowfunc(float t)
{
    static const float ww = std::sqrt(3.0f / 8.0f);
    /* triangular window function: */
#if 0
    if (t <= 0.5f) {
        return ww * 2.0f * t;
    } else {
        return ww * 2.0f * (1.0f - t);
    }
#endif
    /* window function based on cosh: */
#if 1
    static const float acosh2 = std::acosh(2.0f);
    return 0.95f * ww * (2.0f - std::cosh(acosh2 * (2.0f * t - 1.0f)));
#endif
}

void McNames(int n, unsigned char* colormap, float periods)
{
    static const float sqrt3 = std::sqrt(3.0f);
    static const float a12 = std::asin(1.0f / sqrt3);
    static const float a23 = pi / 4.0f;

    for (int i = 0; i < n; i++) {
        float t = 1.0f - i / (n - 1.0f);
        float w = windowfunc(t);
        float tt = (1.0f - t) * sqrt3;
        float ttt = (tt - sqrt3 / 2.0f) * periods * twopi / sqrt3;

        float r0, g0, b0, r1, g1, b1, r2, g2, b2;
        float ag, rd;
        r0 = tt;
        g0 = w * std::cos(ttt);
        b0 = w * std::sin(ttt);
        cart2pol(r0, g0, &ag, &rd);
        pol2cart(ag + a12, rd, &r1, &g1);
        b1 = b0;
        cart2pol(r1, b1, &ag, &rd);
        pol2cart(ag + a23, rd, &r2, &b2);
        g2 = g1;

        colormap[3 * i + 0] = float_to_uchar(clamp(r2, 0.0f, 1.0f));
        colormap[3 * i + 1] = float_to_uchar(clamp(g2, 0.0f, 1.0f));
        colormap[3 * i + 2] = float_to_uchar(clamp(b2, 0.0f, 1.0f));
    }
}

}
