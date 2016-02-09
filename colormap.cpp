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

/* A color triplet class without assumptions about the color space */

class triplet {
public:
    struct {
        union { float x, l,       m, r; };
        union { float y, a, u, c, s, g; };
        union { float z, b, v, h      ; };
    };

    triplet() {}
    triplet(float t0, float t1, float t2) : x(t0), y(t1), z(t2) {}
    triplet(const triplet& t) : x(t.x), y(t.y), z(t.z) {}
};

triplet operator+(triplet t0, triplet t1)
{
    return triplet(t0.x + t1.x, t0.y + t1.y, t0.z + t1.z);
}

triplet operator*(float s, triplet t)
{
    return triplet(s * t.x, s * t.y, s * t.z);
}

/* XYZ and related color spaces helper functions and values */

static float u_prime(triplet xyz)
{
    return 4.0f * xyz.x / (xyz.x + 15.0f * xyz.y + 3.0f * xyz.z);
}

static float v_prime(triplet xyz)
{
    return 9.0f * xyz.y / (xyz.x + 15.0f * xyz.y + 3.0f * xyz.z);
}

static const triplet d65_xyz = triplet(95.047f, 100.000f, 108.883f);
static const float d65_u_prime = u_prime(d65_xyz);
static const float d65_v_prime = v_prime(d65_xyz);

/* Color space conversion: LCH <-> LUV */

static triplet lch_to_luv(triplet lch)
{
    return triplet(lch.l, lch.c * std::cos(lch.h), lch.c * std::sin(lch.h));
}

static triplet luv_to_lch(triplet luv)
{
    triplet lch(luv.l, std::hypot(luv.u, luv.v), std::atan2(luv.v, luv.u));
    if (lch.h < 0.0f)
        lch.h += twopi;
    return lch;
}

static float lch_saturation(float l, float c)
{
    return c / std::max(l, 1e-8f);
}

static float lch_chroma(float l, float s)
{
    return s * l;
}

/* Color space conversion: LUV <-> XYZ */

static triplet luv_to_xyz(triplet luv)
{
    triplet xyz;
    float u_prime = luv.u / (13.0f * luv.l) + d65_u_prime;
    float v_prime = luv.v / (13.0f * luv.l) + d65_v_prime;
    if (luv.l <= 8.0f) {
        xyz.y = d65_xyz.y * luv.l * (3.0f * 3.0f * 3.0f / (29.0f * 29.0f * 29.0f));
    } else {
        float tmp = (luv.l + 16.0f) / 116.0f;
        xyz.y = d65_xyz.y * tmp * tmp * tmp;
    }
    xyz.x = xyz.y * (9.0f * u_prime) / (4.0f * v_prime);
    xyz.z = xyz.y * (12.0f - 3.0f * u_prime - 20.0f * v_prime) / (4.0f * v_prime);
    return xyz;
}

static triplet xyz_to_luv(triplet xyz)
{
    triplet luv;
    float y_ratio = xyz.y / d65_xyz.y;
    if (y_ratio <= (6.0f * 6.0f * 6.0f) / (29.0f * 29.0f * 29.0f)) {
        luv.l = (29.0f * 29.0f * 29.0f) / (3.0f * 3.0f * 3.0f) * y_ratio;
    } else {
        luv.l = 116.0f * std::cbrt(y_ratio) - 16.0f;
    }
    luv.u = 13.0f * luv.l * (u_prime(xyz) - d65_u_prime);
    luv.v = 13.0f * luv.l * (v_prime(xyz) - d65_v_prime);
    return luv;
}

static float luv_saturation(triplet luv)
{
    return lch_saturation(luv.l, std::hypot(luv.u, luv.v));
}

/* Color space conversion: LAB <-> XYZ */

static float lab_invf(float t)
{
    if (t > 6.0f / 29.0f)
        return t * t * t;
    else
        return (3.0f * 6.0f * 6.0f) / (29.0f * 29.0f) * (t - 4.0f / 29.0f);
}

static triplet lab_to_xyz(triplet lab)
{
    float t = (lab.l + 16.0f) / 116.0f;
    return triplet(d65_xyz.x * lab_invf(t + lab.a / 500.0f),
                   d65_xyz.y * lab_invf(t),
                   d65_xyz.z * lab_invf(t - lab.b / 200.0f));
}

static float lab_f(float t)
{
    if (t > (6.0f * 6.0f * 6.0f) / (29.0f * 29.0f * 29.0f))
        return std::cbrt(t);
    else
        return (29.0f * 29.0f) / (3.0f * 6.0f * 6.0f) * t + 4.0f / 29.0f;
}

static triplet xyz_to_lab(triplet xyz)
{
    triplet fxyz = triplet(lab_f(xyz.x / d65_xyz.x), lab_f(xyz.y / d65_xyz.y), lab_f(xyz.z / d65_xyz.z));
    return triplet(116.0f * fxyz.y - 16.0f, 500.0f * (fxyz.x - fxyz.y), 200.0f * (fxyz.y - fxyz.z));
}

/* Color space conversion: RGB <-> XYZ */

static triplet rgb_to_xyz(triplet rgb)
{
    return 100.0f * triplet(
            (0.4124f * rgb.r + 0.3576f * rgb.g + 0.1805f * rgb.b),
            (0.2126f * rgb.r + 0.7152f * rgb.g + 0.0722f * rgb.b),
            (0.0193f * rgb.r + 0.1192f * rgb.g + 0.9505f * rgb.b));
}

static triplet xyz_to_rgb(triplet xyz, bool clamping = true)
{
    triplet rgb = 0.01f * triplet(
            (+3.2406255f * xyz.x - 1.5372080f * xyz.y - 0.4986286f * xyz.z),
            (-0.9689307f * xyz.x + 1.8757561f * xyz.y + 0.0415175f * xyz.z),
            (+0.0557101f * xyz.x - 0.2040211f * xyz.y + 1.0569959f * xyz.z));
    if (clamping) {
        rgb.r = clamp(rgb.r, 0.0f, 1.0f);
        rgb.g = clamp(rgb.g, 0.0f, 1.0f);
        rgb.b = clamp(rgb.b, 0.0f, 1.0f);
    }
    return rgb;
}

/* Color space conversion: RGB <-> sRGB */

static float rgb_to_srgb_helper(float x)
{
    return (x <= 0.0031308f ? (x * 12.92f) : (1.055f * std::pow(x, 1.0f / 2.4f) - 0.055f));
}

static triplet rgb_to_srgb(triplet rgb)
{
    return triplet(rgb_to_srgb_helper(rgb.r), rgb_to_srgb_helper(rgb.g), rgb_to_srgb_helper(rgb.b));
}

static float srgb_to_rgb_helper(float x)
{
    return (x <= 0.04045f ? (x / 12.92f) : std::pow((x + 0.055f) / 1.055f, 2.4f));
}

static triplet srgb_to_rgb(triplet srgb)
{
    return triplet(srgb_to_rgb_helper(srgb.r), srgb_to_rgb_helper(srgb.g), srgb_to_rgb_helper(srgb.b));
}

/* Various helpers */

static float srgb_to_lch_hue(triplet srgb)
{
    return luv_to_lch(xyz_to_luv(rgb_to_xyz(srgb_to_rgb(srgb)))).h;
}

// Compute most saturated color that fits into the sRGB
// cube for the given LCH hue value. This is the core
// of the Wijffelaars paper.
static triplet most_saturated_in_srgb(float lch_hue)
{
    /* Static values, only computed once */
    static float h[] = {
        srgb_to_lch_hue(triplet(1, 0, 0)),
        srgb_to_lch_hue(triplet(1, 1, 0)),
        srgb_to_lch_hue(triplet(0, 1, 0)),
        srgb_to_lch_hue(triplet(0, 1, 1)),
        srgb_to_lch_hue(triplet(0, 0, 1)),
        srgb_to_lch_hue(triplet(1, 0, 1))
    };

    /* RGB values and variable pointers to them */
    int i, j, k;
    if (lch_hue < h[0]) {
        i = 2;
        j = 1;
        k = 0;
    } else if (lch_hue < h[1]) {
        i = 1;
        j = 2;
        k = 0;
    } else if (lch_hue < h[2]) {
        i = 0;
        j = 2;
        k = 1;
    } else if (lch_hue < h[3]) {
        i = 2;
        j = 0;
        k = 1;
    } else if (lch_hue < h[4]) {
        i = 1;
        j = 0;
        k = 2;
    } else if (lch_hue < h[5]) {
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
    float alpha = -std::sin(lch_hue);
    float beta = std::cos(lch_hue);
    float T = alpha * d65_u_prime + beta * d65_v_prime;
    srgb[j] = 0.0f;
    srgb[k] = 1.0f;
    float q0 = T * (M[0][k] + 15.0f * M[1][k] + 3.0f * M[2][k]) - (4.0f * alpha * M[0][k] + 9.0f * beta * M[1][k]);
    float q1 = T * (M[0][i] + 15.0f * M[1][i] + 3.0f * M[2][i]) - (4.0f * alpha * M[0][i] + 9.0f * beta * M[1][i]);
    srgb[i] = rgb_to_srgb_helper(clamp(- q0 / q1, 0.0f, 1.0f));

    /* Convert back to LUV */
    return xyz_to_luv(rgb_to_xyz(srgb_to_rgb(triplet(srgb[0], srgb[1], srgb[2]))));
}

static float s_max(float l, float h)
{
    triplet pmid = most_saturated_in_srgb(h);
    triplet pend = triplet(0.0f, 0.0f, 0.0f);
    if (l > pmid.l)
        pend.l = 100.0f;
    float alpha = (pend.l - l) / (pend.l - pmid.l);
    float pmids = luv_saturation(pmid);
    float pends = luv_saturation(pend);
    return alpha * (pmids - pends) + pends;
}

static triplet get_bright_point()
{
    static triplet pb(-1.0f, -1.0f, -1.0f);
    if (pb.l < 0.0f) {
        pb = xyz_to_luv(rgb_to_xyz(triplet(1.0f, 1.0f, 0.0f)));
    }
    return pb;
}

static float mix_hue(float alpha, float h0, float h1)
{
    float M = std::fmod(pi + h1 - h0, twopi) - pi;
    return std::fmod(h0 + alpha * M, twopi);
}

static void get_color_points(float hue, float saturation, float warmth,
        triplet pb, float pb_hue, float pb_saturation,
        triplet* p0, triplet* p1, triplet* p2,
        triplet* q0, triplet* q1, triplet* q2)
{
    *p0 = lch_to_luv(triplet(0.0f, 0.0f, hue));
    *p1 = most_saturated_in_srgb(hue);
    triplet p2_lch;
    p2_lch.l = (1.0f - warmth) * 100.0f + warmth * pb.l;
    p2_lch.h = mix_hue(warmth, hue, pb_hue);
    p2_lch.c = lch_chroma(p2_lch.l, std::min(s_max(p2_lch.l, p2_lch.h), warmth * saturation * pb_saturation));
    *p2 = lch_to_luv(p2_lch);
    *q0 = (1.0f - saturation) * (*p0) + saturation * (*p1);
    *q2 = (1.0f - saturation) * (*p2) + saturation * (*p1);
    *q1 = 0.5f * ((*q0) + (*q2));
}

static triplet b(triplet b0, triplet b1, triplet b2, float t)
{
    float a = (1.0f - t) * (1.0f - t);
    float b = 2.0f * (1.0f - t) * t;
    float c = t * t;
    return a * b0 + b * b1 + c * b2;
}

static float inv_b(float b0, float b1, float b2, float v)
{
    return (b0 - b1 + std::sqrt(std::max(b1 * b1 - b0 * b2 + (b0 - 2.0f * b1 + b2) * v, 0.0f)))
        / (b0 - 2.0f * b1 + b2);
}

static triplet get_colormap_entry(float t,
        triplet p0, triplet p2,
        triplet q0, triplet q1, triplet q2,
        float contrast, float brightness)
{
    float l = 125 - 125 * std::pow(0.2f, (1.0f - contrast) * brightness + t * contrast);
    float T = (l <= q1.l ? 0.5f * inv_b(p0.l, q0.l, q1.l, l) : 0.5f * inv_b(q1.l, q2.l, p2.l, l) + 0.5f);
    return (T <= 0.5f ? b(p0, q0, q1, 2.0f * T) : b(q1, q2, p2, 2.0f * (T - 0.5f)));
}

static void luv_to_colormap(triplet luv, unsigned char* colormap)
{
    triplet srgb = rgb_to_srgb(xyz_to_rgb(luv_to_xyz(luv)));
    colormap[0] = float_to_uchar(srgb.r);
    colormap[1] = float_to_uchar(srgb.g);
    colormap[2] = float_to_uchar(srgb.b);
}

/* Brewer-like color maps */

float BrewerSequentialDefaultContrastForSmallN(int n)
{
    return std::min(0.88f, 0.34f + 0.06f * n);
}

void BrewerSequential(int n, unsigned char* colormap, float hue,
        float contrast, float saturation, float brightness, float warmth)
{
    triplet pb, p0, p1, p2, q0, q1, q2;
    pb = get_bright_point();
    triplet pb_lch = luv_to_lch(pb);
    float pbs = lch_saturation(pb_lch.l, pb_lch.c);
    get_color_points(hue, saturation, warmth, pb, pb_lch.h, pbs, &p0, &p1, &p2, &q0, &q1, &q2);

    for (int i = 0; i < n; i++) {
        float t = i / (n - 1.0f);
        triplet c = get_colormap_entry(t, p0, p2, q0, q1, q2, contrast, brightness);
        luv_to_colormap(c, colormap + 3 * i);
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

    triplet pb;
    triplet p00, p01, p02, q00, q01, q02;
    triplet p10, p11, p12, q10, q11, q12;
    pb = get_bright_point();
    triplet pb_lch = luv_to_lch(pb);
    float pbs = lch_saturation(pb_lch.l, pb_lch.c);
    get_color_points(hue,  saturation, warmth, pb, pb_lch.h, pbs, &p00, &p01, &p02, &q00, &q01, &q02);
    get_color_points(hue1, saturation, warmth, pb, pb_lch.h, pbs, &p10, &p11, &p12, &q10, &q11, &q12);

    for (int i = 0; i < n; i++) {
        triplet c;
        if (n % 2 == 1 && i == n / 2) {
            // compute neutral color in the middle of the map
            triplet c0 = get_colormap_entry(1.0f, p00, p02, q00, q01, q02, contrast, brightness);
            triplet c1 = get_colormap_entry(1.0f, p10, p12, q10, q11, q12, contrast, brightness);
            if (n <= 9) {
                // for discrete color maps, use an extra neutral color
                float c0s = luv_saturation(c0);
                float c1s = luv_saturation(c1);
                float sn = 0.5f * (c0s + c1s) * warmth;
                c.l = 0.5f * (c0.l + c1.l);
                float cc = lch_chroma(c.l, std::min(s_max(c.l, pb_lch.h), sn));
                c = lch_to_luv(triplet(c.l, cc, pb_lch.h));
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
        luv_to_colormap(c, colormap + 3 * i);
    }
}

void BrewerQualitative(int n, unsigned char* colormap, float hue, float divergence,
        float contrast, float saturation, float brightness)
{
    // Get all information about yellow
    static triplet ylch(-1.0f, -1.0f, -1.0f);
    if (ylch.l < 0.0f) {
        ylch = luv_to_lch(xyz_to_luv(rgb_to_xyz(triplet(1.0f, 1.0f, 0.0f))));
    }

    // Get saturation of red (maximum possible saturation)
    static float rs = -1.0f;
    if (rs < 0.0f) {
        triplet rluv = xyz_to_luv(rgb_to_xyz(triplet(1.0f, 0.0f, 0.0f)));
        rs = luv_saturation(rluv);
    }

    // Derive parameters of the method
    float eps = hue / twopi;
    float r = divergence / twopi;
    float l0 = brightness * ylch.l;
    float l1 = (1.0f - contrast) * l0;

    // Generate colors
    for (int i = 0; i < n; i++) {
        float t = i / (n - 1.0f);
        float ch = std::fmod(twopi * (eps + t * r), twopi);
        float alpha = hue_diff(ch, ylch.h) / pi;
        float cl = (1.0f - alpha) * l0 + alpha * l1;
        float cs = std::min(s_max(cl, ch), saturation * rs);
        triplet c = lch_to_luv(triplet(cl, lch_chroma(cl, cs), ch));
        luv_to_colormap(c, colormap + 3 * i);
    }
}

/* Isoluminant */

static void lch_to_colormap(triplet lch, unsigned char* colormap)
{
    triplet srgb = rgb_to_srgb(xyz_to_rgb(luv_to_xyz(lch_to_luv(lch))));
    colormap[0] = float_to_uchar(srgb.r);
    colormap[1] = float_to_uchar(srgb.g);
    colormap[2] = float_to_uchar(srgb.b);
}

void IsoluminantSequential(int n, unsigned char* colormap,
        float luminance, float saturation, float hue)
{
    triplet lch;
    lch.l = luminance * 100.0f;
    lch.h = hue;
    for (int i = 0; i < n; i++) {
        float t = i / (n - 1.0f);
        float s = saturation * 5.0f * (1.0f - t);
        lch.c = lch_chroma(lch.l, s);
        lch_to_colormap(lch, colormap + 3 * i);
    }
}

void IsoluminantDiverging(int n, unsigned char* colormap,
        float luminance, float saturation, float hue, float divergence)
{
    triplet lch;
    lch.l = luminance * 100.0f;
    for (int i = 0; i < n; i++) {
        float t = i / (n - 1.0f);
        float s = saturation * 5.0f * (t <= 0.5f ? 2.0f * (0.5f - t) : 2.0f * (t - 0.5f));
        lch.c = lch_chroma(lch.l, s);
        lch.h = (t <= 0.5f ? hue : hue + divergence);
        lch_to_colormap(lch, colormap + 3 * i);
    }
}

void IsoluminantQualitative(int n, unsigned char* colormap,
        float luminance, float saturation, float hue, float divergence)
{
    triplet lch;
    lch.l = luminance * 100.0f;
    lch.c = lch_chroma(lch.l, saturation * 5.0f);
    for (int i = 0; i < n; i++) {
        float t = i / (n - 1.0f);
        lch.h = hue + t * divergence;
        lch_to_colormap(lch, colormap + 3 * i);
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

static triplet lab_to_msh(triplet lab)
{
    triplet msh;
    msh.m = std::sqrt(lab.l * lab.l + lab.a * lab.a + lab.b * lab.b);
    msh.s = (msh.m > 0.001f) ? std::acos(lab.l / msh.m) : 0.0f;
    msh.h = (msh.s > 0.001f) ? std::atan2(lab.b, lab.a) : 0.0f;
    return msh;
}

static triplet msh_to_lab(triplet msh)
{
    return triplet(
            msh.m * std::cos(msh.s),
            msh.m * std::sin(msh.s) * std::cos(msh.h),
            msh.m * std::sin(msh.s) * std::sin(msh.h));
}

static float adjust_hue(triplet msh, float unsaturated_m)
{
    if (msh.m >= unsaturated_m - 0.1f) {
        return msh.h;
    } else {
        float hue_spin = msh.s * std::sqrt(unsaturated_m * unsaturated_m - msh.m * msh.m)
            / (msh.m * std::sin(msh.s));
        if (msh.h > -pi / 3.0f)
            return msh.h + hue_spin;
        else
            return msh.h - hue_spin;
    }
}

void Moreland(int n, unsigned char* colormap,
        unsigned char sr0, unsigned char sg0, unsigned char sb0,
        unsigned char sr1, unsigned char sg1, unsigned char sb1)
{
    triplet omsh0 = lab_to_msh(xyz_to_lab(rgb_to_xyz(srgb_to_rgb(triplet(
                            uchar_to_float(sr0), uchar_to_float(sg0), uchar_to_float(sb0))))));
    triplet omsh1 = lab_to_msh(xyz_to_lab(rgb_to_xyz(srgb_to_rgb(triplet(
                            uchar_to_float(sr1), uchar_to_float(sg1), uchar_to_float(sb1))))));
    bool place_white = (omsh0.s >= 0.05f && omsh1.s >= 0.05f && hue_diff(omsh0.h, omsh1.h) > pi / 3.0f);
    float mmid = std::max(std::max(omsh0.m, omsh1.m), 88.0f);

    for (int i = 0; i < n; i++) {
        triplet msh0 = omsh0;
        triplet msh1 = omsh1;
        float t = i / (n - 1.0f);
        if (place_white) {
            if (t < 0.5f) {
                msh1.m = mmid;
                msh1.s = 0.0f;
                msh1.h = 0.0f;
                t *= 2.0f;
            } else {
                msh0.m = mmid;
                msh0.s = 0.0f;
                msh0.h = 0.0f;
                t = 2.0f * t - 1.0f;
            }
        }
        if (msh0.s < 0.05f && msh1.s >= 0.05f) {
            msh0.h = adjust_hue(msh1, msh0.m);
        } else if (msh0.s >= 0.05f && msh1.s < 0.05f) {
            msh1.h = adjust_hue(msh0, msh1.m);
        }
        triplet msh = (1.0f - t) * msh0 + t * msh1;
        triplet srgb = rgb_to_srgb(xyz_to_rgb(lab_to_xyz(msh_to_lab(msh))));
        colormap[3 * i + 0] = float_to_uchar(srgb.r);
        colormap[3 * i + 1] = float_to_uchar(srgb.g);
        colormap[3 * i + 2] = float_to_uchar(srgb.b);
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
