/*
 * Copyright (C) 2015, 2016, 2017, 2018, 2019
 * Computer Graphics Group, University of Siegen
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
 * Usage:
 * - Decide which type of color map you need and how many colors the map should
 *   contain.
 * - Allocate memory for you color map (3 * unsigned char for each color entry).
 * - Call the function that generates your color map.
 *   The return value is always the number of colors that had to be clipped
 *   to fit into sRGB; you want to keep that number low by adjusting parameters.
 *
 * All colors are represented as unsigned char sRGB triplets, with each value in
 * [0,255].
 */

namespace ColorMap {

/*
 * Brewer-like color maps, as described in
 * M. Wijffelaars, R. Vliegen, J.J. van Wijk, E.-J. van der Linden. Generating
 * color palettes using intuitive parameters. In Computer Graphics Forum, vol. 27,
 * no. 3, pp. 743-750, 2008.
 */

// Create a sequential color map with n colors of the given hue in [0,2*PI].

const float BrewerSequentialDefaultHue = 4.18879020479f; // 240 deg
const float BrewerSequentialDefaultContrast = 0.88f;
float BrewerSequentialDefaultContrastForSmallN(int n); // only for discrete color maps, i.e. n <= 9
const float BrewerSequentialDefaultSaturation = 0.6f;
const float BrewerSequentialDefaultBrightness = 0.75f;
const float BrewerSequentialDefaultWarmth = 0.15f;

int BrewerSequential(int n, unsigned char* srgb_colormap,
        float hue = BrewerSequentialDefaultHue,
        float contrast = BrewerSequentialDefaultContrast,
        float saturation = BrewerSequentialDefaultSaturation,
        float brightness = BrewerSequentialDefaultBrightness,
        float warmth = BrewerSequentialDefaultWarmth);

// Create a diverging color map with n colors. Half of them will have the given
// hue (in [0,2*PI]), the other half will have a hue that has the distance given
// by divergence (in [0,2*PI]) to that hue, and they will meet in the middle at
// a neutral color.

const float BrewerDivergingDefaultHue = 4.18879020479f; // 240 deg
const float BrewerDivergingDefaultDivergence = 4.18879020479f; // 240 deg = 2/3 * 2PI
const float BrewerDivergingDefaultContrast = 0.88f;
float BrewerDivergingDefaultContrastForSmallN(int n); // only for discrete color maps, i.e. n <= 9
const float BrewerDivergingDefaultSaturation = 0.6f;
const float BrewerDivergingDefaultBrightness = 0.75f;
const float BrewerDivergingDefaultWarmth = 0.15f;

int BrewerDiverging(int n, unsigned char* srgb_colormap,
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
const float BrewerQualitativeDefaultBrightness = 0.8f;

int BrewerQualitative(int n, unsigned char* colormap,
        float hue = BrewerQualitativeDefaultHue,
        float divergence = BrewerQualitativeDefaultDivergence,
        float contrast = BrewerQualitativeDefaultContrast,
        float saturation = BrewerQualitativeDefaultSaturation,
        float brightness = BrewerQualitativeDefaultBrightness);

/*
 * Perceptually linear (PL) color maps.
 *
 * These are computed in CIELUV/LCH to achieve perceptual linearity.
 * One or two of lightness, saturation, and hue are changed while the other(s)
 * stay constant.
 *
 * For example, color maps of constant lightness are useful for 3D surfaces
 * because they do not interfere with additional shading.
 */

/* Sequential perceptually linear maps */

// Varying lightness

const float PLSequentialLightnessDefaultSaturation = 0.45f;
const float PLSequentialLightnessDefaultHue = 0.349065850399f; // 20 deg

int PLSequentialLightness(int n, unsigned char* colormap,
        float saturation = PLSequentialLightnessDefaultSaturation,
        float hue = PLSequentialLightnessDefaultHue);

// Varying saturation

const float PLSequentialSaturationDefaultLightness = 0.5f;
const float PLSequentialSaturationDefaultSaturation = 0.45f;
const float PLSequentialSaturationDefaultHue = 0.349065850399f; // 20 deg

int PLSequentialSaturation(int n, unsigned char* colormap,
        float lightness = PLSequentialSaturationDefaultLightness,
        float saturation = PLSequentialSaturationDefaultSaturation,
        float hue = PLSequentialSaturationDefaultHue);

// Varying hue (through all colors, rainbow-like)

const float PLSequentialRainbowDefaultHue = 0.0f;
const float PLSequentialRainbowDefaultRotations = -1.5f;
const float PLSequentialRainbowDefaultSaturation = 1.1f;

int PLSequentialRainbow(int n, unsigned char* colormap,
        float hue = PLSequentialRainbowDefaultHue,
        float rotations = PLSequentialRainbowDefaultRotations,
        float saturation = PLSequentialRainbowDefaultSaturation);

// Varying hue (through physically plausible black body colors at increasing
// temperatures).
// The defaults are chosen so that we start at red and arrive at the D65 white
// point (6500 K), thus excluding the blue colors that occur at higher
// temperatures.
const float PLSequentialBlackBodyDefaultTemperature = 250.0f;
const float PLSequentialBlackBodyDefaultRange = 6250.0f;
const float PLSequentialBlackBodyDefaultSaturation = 2.3f;

int PLSequentialBlackBody(int n, unsigned char* colormap,
        float temperature = PLSequentialBlackBodyDefaultTemperature,
        float range = PLSequentialBlackBodyDefaultRange,
        float saturation = PLSequentialBlackBodyDefaultSaturation);

// Varying hue (user definable)
const float PLSequentialMultiHueDefaultL0 = 0.01f; // lightness at start
const float PLSequentialMultiHueDefaultS0 = 0.0f; // saturation at start
const float PLSequentialMultiHueDefaultL1 = 100.0f; // lightness at end
const float PLSequentialMultiHueDefaultS1 = 0.0f; // saturation at end
const float PLSequentialMultiHueDefaultS05 = 1.0f; // saturation in the middle
const int PLSequentialMultiHueDefaultHues = 1; // number of hues defined in the following lists
const float PLSequentialMultiHueDefaultHueValues[] = { 0.0f }; // hues values in radians in [0,2pi]
const float PLSequentialMultiHueDefaultHuePositions[] = { 0.5f }; // hue positions in [0,1] sorted in ascending order

int PLSequentialMultiHue(int n, unsigned char* colormap,
        int hues = PLSequentialMultiHueDefaultHues,
        const float* hue_values = PLSequentialMultiHueDefaultHueValues,
        const float* hue_positions = PLSequentialMultiHueDefaultHuePositions,
        float l0 = PLSequentialMultiHueDefaultL0,
        float s0 = PLSequentialMultiHueDefaultS0,
        float l1 = PLSequentialMultiHueDefaultL1,
        float s1 = PLSequentialMultiHueDefaultS1,
        float s05 = PLSequentialMultiHueDefaultS05);

/* Diverging perceptually linear maps */

// Varying lightness

const float PLDivergingLightnessDefaultLightness = 0.05f;
const float PLDivergingLightnessDefaultSaturation = 0.45f;
const float PLDivergingLightnessDefaultHue = 0.349065850399f; // 20 deg
const float PLDivergingLightnessDefaultDivergence = 4.18879020479f; // 2/3 * 2PI

int PLDivergingLightness(int n, unsigned char* colormap,
        float lightness = PLDivergingLightnessDefaultLightness,
        float saturation = PLDivergingLightnessDefaultSaturation,
        float hue = PLDivergingLightnessDefaultHue,
        float divergence = PLDivergingLightnessDefaultDivergence);

// Varying saturation

const float PLDivergingSaturationDefaultLightness = 0.5f;
const float PLDivergingSaturationDefaultSaturation = 0.45f;
const float PLDivergingSaturationDefaultHue = 0.349065850399f; // 20 deg
const float PLDivergingSaturationDefaultDivergence = 4.18879020479f; // 2/3 * 2PI

int PLDivergingSaturation(int n, unsigned char* colormap,
        float lightness = PLDivergingSaturationDefaultLightness,
        float saturation = PLDivergingSaturationDefaultSaturation,
        float hue = PLDivergingSaturationDefaultHue,
        float divergence = PLDivergingSaturationDefaultDivergence);

/* Qualitative perceptually linear maps */

const float PLQualitativeHueDefaultHue = 0.0f;
const float PLQualitativeHueDefaultDivergence = 4.18879020479f; // 2/3 * 2PI
const float PLQualitativeHueDefaultLightness = 0.55f;
const float PLQualitativeHueDefaultSaturation = 0.22f;

int PLQualitativeHue(int n, unsigned char* colormap,
        float hue = PLQualitativeHueDefaultHue,
        float divergence = PLQualitativeHueDefaultDivergence,
        float lightness = PLQualitativeHueDefaultLightness,
        float saturation = PLQualitativeHueDefaultSaturation);

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

/*
 * Moreland color maps, as described in
 * K. Moreland, Diverging Color Maps for Scientific Visualization, Proc. Int.
 * Symp. Visual Computing, December 2009, DOI 10.1007/978-3-642-10520-3_9.
 */

// Create a Moreland colormap with n colors. Specify the two endpoints
// of the colormap as sRGB colors; all intermediate colors will be generated.

const unsigned char MorelandDefaultR0 = 180;
const unsigned char MorelandDefaultG0 = 4;
const unsigned char MorelandDefaultB0 = 38;
const unsigned char MorelandDefaultR1 = 59;
const unsigned char MorelandDefaultG1 = 76;
const unsigned char MorelandDefaultB1 = 192;

int Moreland(int n, unsigned char* colormap,
        unsigned char sr0 = MorelandDefaultR0,
        unsigned char sg0 = MorelandDefaultG0,
        unsigned char sb0 = MorelandDefaultB0,
        unsigned char sr1 = MorelandDefaultR1,
        unsigned char sg1 = MorelandDefaultG1,
        unsigned char sb1 = MorelandDefaultB1);

/*
 * McNames color maps, as described in
 * J. McNames, An Effective Color Scale for Simultaneous Color and Gray-Scale Publications,
 * IEEE Signal Processing Magazine 23(1), January 2006, DOI 10.1109/MSP.2006.1593340.
 *
 * Note: Use CubeHelix instead! The McNames color maps are perceptually not linear in luminance!
 */

// Create a McNames colormap with n colors. Specify the number of
// periods.

const float McNamesDefaultPeriods = 2.0f;

int McNames(int n, unsigned char* colormap,
        float periods = McNamesDefaultPeriods);

}

#endif
