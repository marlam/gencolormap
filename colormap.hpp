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

/*
 * Brewer-like color maps, as described in
 * M. Wijffelaars, R. Vliegen, J.J. van Wijk, E.-J. van der Linden. Generating
 * color palettes using intuitive parameters. In Computer Graphics Forum, vol. 27,
 * no. 3, pp. 743-750, 2008.
 */

// Create a sequential color map with n colors of the given hue in [0,2*PI].

const float BrewerSequentialDefaultHue = 0.0;
const float BrewerSequentialDefaultContrast = 0.88f;
float BrewerSequentialDefaultContrastForSmallN(int n); // only for discrete color maps, i.e. n <= 9
const float BrewerSequentialDefaultSaturation = 0.6f;
const float BrewerSequentialDefaultBrightness = 0.75f;
const float BrewerSequentialDefaultWarmth = 0.15f;

void BrewerSequential(int n, unsigned char* srgb_colormap,
        float hue = BrewerSequentialDefaultHue,
        float contrast = BrewerSequentialDefaultContrast,
        float saturation = BrewerSequentialDefaultSaturation,
        float brightness = BrewerSequentialDefaultBrightness,
        float warmth = BrewerSequentialDefaultWarmth);

// Create a diverging color map with n colors. Half of them will have the given
// hue (in [0,2*PI]), the other half will have a hue that has the distance given
// by divergence (in [0,2*PI]) to that hue, and they will meet in the middle at
// a neutral color.

const float BrewerDivergingDefaultHue = 0.0;
const float BrewerDivergingDefaultDivergence = 4.18879020479f; // 2/3 * 2PI
const float BrewerDivergingDefaultContrast = 0.88f;
float BrewerDivergingDefaultContrastForSmallN(int n); // only for discrete color maps, i.e. n <= 9
const float BrewerDivergingDefaultSaturation = 0.6f;
const float BrewerDivergingDefaultBrightness = 0.75f;
const float BrewerDivergingDefaultWarmth = 0.15f;

void BrewerDiverging(int n, unsigned char* srgb_colormap,
        float hue = BrewerDivergingDefaultHue,
        float divergence = BrewerDivergingDefaultDivergence,
        float contrast = BrewerDivergingDefaultContrast,
        float saturation = BrewerDivergingDefaultSaturation,
        float brightness = BrewerDivergingDefaultBrightness,
        float warmth = BrewerDivergingDefaultWarmth);

// Create a qualitative color map with n colors. The colors will have the same
// saturation; lightness and hue will differ. The parameter hue sets the hue of
// the first color, and the parameter divergence defines the hue range starting
// from that hue that can be used for the colors.

const float BrewerQualitativeDefaultHue = 0.0f;
const float BrewerQualitativeDefaultDivergence = 4.18879020479f; // 2/3 * 2PI
const float BrewerQualitativeDefaultContrast = 0.5f;
const float BrewerQualitativeDefaultSaturation = 0.5f;
const float BrewerQualitativeDefaultBrightness = 1.0f;

void BrewerQualitative(int n, unsigned char* colormap,
        float hue = BrewerQualitativeDefaultHue,
        float divergence = BrewerQualitativeDefaultDivergence,
        float contrast = BrewerQualitativeDefaultContrast,
        float saturation = BrewerQualitativeDefaultSaturation,
        float brightness = BrewerQualitativeDefaultBrightness);

/*
 * CubeHelix color maps, as described in
 * Green, D. A., 2011, A colour scheme for the display of astronomical intensity
 * images, Bulletin of the Astronomical Society of India, 39, 289.
 */

// Create a CubeHelix colormap with n colors. The parameter hue (in [0,2*PI])
// sets the hue of the first color. The parameter rot sets the number of
// rotations. It can be negative for backwards rotation. The saturation parameter
// determines the saturation of the colors; higher values may lead to clipping
// of colors in the sRGB space. The gamma parameter sets optional gamma correction.
// The return value is the number of colors that had to be clipped.

const float CubeHelixDefaultHue = 0.523598775598f; // 1/12 * 2PI
const float CubeHelixDefaultRotations = -1.5f;
const float CubeHelixDefaultSaturation = 1.2f;
const float CubeHelixDefaultGamma = 1.0f;

int CubeHelix(int n, unsigned char* colormap,
        float hue = CubeHelixDefaultHue,
        float rotations = CubeHelixDefaultRotations,
        float saturation = CubeHelixDefaultSaturation,
        float gamma = CubeHelixDefaultGamma);
}

#endif
