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
 * Perceptually unifrom (PU) color maps.
 *
 * These are computed in CIELUV/LCH to achieve approximate perceptual uniformity.
 * One or two of lightness, saturation, and hue are changed while the other(s)
 * stay constant.
 *
 * For example, color maps of constant lightness are useful for 3D surfaces
 * because they do not interfere with additional shading.
 */

/* Sequential perceptually uniform maps */

// Varying lightness

const float PUSequentialLightnessDefaultLightnessRange = 0.95f;
const float PUSequentialLightnessDefaultSaturationRange = 0.95f;
const float PUSequentialLightnessDefaultSaturation = 0.45f;
const float PUSequentialLightnessDefaultHue = 0.349065850399f; // 20 deg

int PUSequentialLightness(int n, unsigned char* colormap,
        float lightness_range = PUSequentialLightnessDefaultLightnessRange,
        float saturation_range = PUSequentialLightnessDefaultSaturationRange,
        float saturation = PUSequentialLightnessDefaultSaturation,
        float hue = PUSequentialLightnessDefaultHue);

// Varying saturation

const float PUSequentialSaturationDefaultSaturationRange = PUSequentialLightnessDefaultSaturationRange;
const float PUSequentialSaturationDefaultLightness = 0.5f;
const float PUSequentialSaturationDefaultSaturation = PUSequentialLightnessDefaultSaturation;
const float PUSequentialSaturationDefaultHue = 0.349065850399f; // 20 deg

int PUSequentialSaturation(int n, unsigned char* colormap,
        float saturation_range = PUSequentialSaturationDefaultSaturationRange,
        float lightness = PUSequentialSaturationDefaultLightness,
        float saturation = PUSequentialSaturationDefaultSaturation,
        float hue = PUSequentialSaturationDefaultHue);

// Varying hue (through all colors, rainbow-like)

const float PUSequentialRainbowDefaultLightnessRange = PUSequentialLightnessDefaultLightnessRange;
const float PUSequentialRainbowDefaultSaturationRange = PUSequentialLightnessDefaultSaturationRange;
const float PUSequentialRainbowDefaultHue = 0.0f;
const float PUSequentialRainbowDefaultRotations = -1.5f;
const float PUSequentialRainbowDefaultSaturation = 1.1f;

int PUSequentialRainbow(int n, unsigned char* colormap,
        float lightness_range = PUSequentialRainbowDefaultLightnessRange,
        float saturation_range = PUSequentialRainbowDefaultSaturationRange,
        float hue = PUSequentialRainbowDefaultHue,
        float rotations = PUSequentialRainbowDefaultRotations,
        float saturation = PUSequentialRainbowDefaultSaturation);

// Varying hue (through physically plausible black body colors at increasing
// temperatures).
// The defaults are chosen so that we start at red and arrive at the D65 white
// point (6500 K), thus excluding the blue colors that occur at higher
// temperatures.
const float PUSequentialBlackBodyDefaultTemperature = 250.0f;
const float PUSequentialBlackBodyDefaultRange = 6250.0f;
const float PUSequentialBlackBodyDefaultSaturation = 2.3f;

int PUSequentialBlackBody(int n, unsigned char* colormap,
        float temperature = PUSequentialBlackBodyDefaultTemperature,
        float range = PUSequentialBlackBodyDefaultRange,
        float saturation = PUSequentialBlackBodyDefaultSaturation);

// Varying hue (user definable)
const float PUSequentialMultiHueDefaultLightnessRange = PUSequentialLightnessDefaultLightnessRange;
const float PUSequentialMultiHueDefaultSaturationRange = PUSequentialSaturationDefaultSaturationRange;
const float PUSequentialMultiHueDefaultSaturation = 0.38f;
const int PUSequentialMultiHueDefaultHues = 2; // number of hues defined in the following lists
const float PUSequentialMultiHueDefaultHueValues[] = { 0.0f, 1.0471975512f }; // hues values in radians in [0,2pi]
const float PUSequentialMultiHueDefaultHuePositions[] = { 0.25f, 0.75f }; // hue positions in [0,1] sorted in ascending order

int PUSequentialMultiHue(int n, unsigned char* colormap,
        float lightness_range = PUSequentialMultiHueDefaultLightnessRange,
        float saturation_range = PUSequentialMultiHueDefaultSaturationRange,
        float saturation = PUSequentialMultiHueDefaultSaturation,
        int hues = PUSequentialMultiHueDefaultHues,
        const float* hue_values = PUSequentialMultiHueDefaultHueValues,
        const float* hue_positions = PUSequentialMultiHueDefaultHuePositions);

/* Diverging perceptually uniform maps */

// Varying lightness

const float PUDivergingLightnessDefaultLightnessRange = PUSequentialLightnessDefaultLightnessRange;
const float PUDivergingLightnessDefaultSaturationRange = PUSequentialLightnessDefaultSaturationRange;
const float PUDivergingLightnessDefaultSaturation = PUSequentialLightnessDefaultSaturation;
const float PUDivergingLightnessDefaultHue = 0.349065850399f; // 20 deg
const float PUDivergingLightnessDefaultDivergence = 4.18879020479f; // 2/3 * 2PI

int PUDivergingLightness(int n, unsigned char* colormap,
        float lightness_range = PUDivergingLightnessDefaultLightnessRange,
        float saturation_range = PUDivergingLightnessDefaultSaturationRange,
        float saturation = PUDivergingLightnessDefaultSaturation,
        float hue = PUDivergingLightnessDefaultHue,
        float divergence = PUDivergingLightnessDefaultDivergence);

// Varying saturation

const float PUDivergingSaturationDefaultSaturationRange = PUSequentialSaturationDefaultSaturationRange;
const float PUDivergingSaturationDefaultLightness = 0.5f;
const float PUDivergingSaturationDefaultSaturation = 0.45f;
const float PUDivergingSaturationDefaultHue = 0.349065850399f; // 20 deg
const float PUDivergingSaturationDefaultDivergence = 4.18879020479f; // 2/3 * 2PI

int PUDivergingSaturation(int n, unsigned char* colormap,
        float saturation_range = PUSequentialSaturationDefaultSaturationRange,
        float lightness = PUDivergingSaturationDefaultLightness,
        float saturation = PUDivergingSaturationDefaultSaturation,
        float hue = PUDivergingSaturationDefaultHue,
        float divergence = PUDivergingSaturationDefaultDivergence);

/* Qualitative perceptually uniform maps */

const float PUQualitativeHueDefaultHue = 0.0f;
const float PUQualitativeHueDefaultDivergence = 4.18879020479f; // 2/3 * 2PI
const float PUQualitativeHueDefaultLightness = 0.55f;
const float PUQualitativeHueDefaultSaturation = 0.22f;

int PUQualitativeHue(int n, unsigned char* colormap,
        float hue = PUQualitativeHueDefaultHue,
        float divergence = PUQualitativeHueDefaultDivergence,
        float lightness = PUQualitativeHueDefaultLightness,
        float saturation = PUQualitativeHueDefaultSaturation);

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
