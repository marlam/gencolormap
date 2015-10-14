/*
 * Copyright (C) 2015 Computer Graphics Group, University of Siegen
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

#ifndef COLORMAP_HPP
#define COLORMAP_HPP

/* Generate color maps for scientific visualization purposes.
 *
 * This implements the methods described in
 * "Generating Color Palettes using Intuitive Parameters" by
 * Martijn Wijffelaars, Roel Vliegen, Jarke J. van Wijk and Erik-Jan van der
 * Linden, Eurographics/IEEE-VGTC Symposium on Visualization 2008
 *
 * Usage:
 *
 * Decide which type of color map you need and how many colors the map should
 * contain:
 * - For a continuous range of values (e.g. temperature, size, ...):
 *   Sequential map with a single hue, at least 200 colors
 * - For a continuous of values around a neutral middle (e.g. deviation from an
 *   ideal value):
 *   Diverging map, composed of two sequential maps with a neutral color in the
 *   middle, at least 200 colors
 *
 * Allocate memory for your color map, and call the function that generates your
 * map. All colors are represented as unsigned char sRGB triplets, with each
 * value in [0,255].
 */

namespace ColorMap {

const float DefaultContrast = 0.88f;
float DefaultContrastForSmallN(int n); // only for discrete color maps, i.e. n <= 9
const float DefaultSaturation = 0.6f;
const float DefaultBrightness = 0.75f;
const float DefaultWarmth = 0.15f;

// Create a sequential color map with n colors of the given hue in [0,2*PI].
void Sequential(int n, unsigned char* srgb_colormap, float hue,
        float contrast = DefaultContrast,
        float saturation = DefaultSaturation,
        float brightness = DefaultBrightness,
        float warmth = DefaultWarmth);

// Create a diverging color map with n colors. Half of them will have hue0 (in
// [0,2*PI]), the other half will have a hue that has the distance given by
// divergence (in [0,2*PI]) to hue0, and they will meet in the middle at a
// neutral color.
void Diverging(int n, unsigned char* srgb_colormap, float hue0, float divergence,
        float contrast = DefaultContrast,
        float saturation = DefaultSaturation,
        float brightness = DefaultBrightness,
        float warmth = DefaultWarmth);

const float DefaultQualitativeDivergence = 6.2831853071795864769; // 2*PI
const float DefaultQualitativeContrast = 0.5f;
const float DefaultQualitativeSaturation = 0.5f;
const float DefaultQualitativeBrightness = 1.0f;

// Create a qualitative color map with n colors. The colors will have the same
// saturation; lightness and hue will differ. The parameter hue0 sets the hue of
// the first color, and the parameter divergence defines the hue range starting
// from hue0 that can be used for the colors.
void Qualitative(int n, unsigned char* colormap, float hue0,
        float divergence = DefaultQualitativeDivergence,
        float contrast = DefaultQualitativeContrast,
        float saturation = DefaultQualitativeSaturation,
        float brightness = DefaultQualitativeBrightness);

}

#endif
