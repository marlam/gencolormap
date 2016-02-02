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

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <getopt.h>
extern char *optarg;
extern int optind;

#include "colormap.hpp"

enum type {
    brewer_sequential = 0,
    brewer_diverging = 1,
    brewer_qualitative = 2,
    cubehelix = 3
};

int main(int argc, char* argv[])
{
    bool print_version = false;
    bool print_help = false;
    int type = -1;
    int n = -1;
    float hue = -1.0f;
    float divergence = -1.0f;
    float contrast = -1.0f;
    float saturation = -1.0f;
    float brightness = -1.0f;
    float warmth = -1.0f;
    float rotations = NAN;
    float gamma = -1.0f;
    struct option options[] = {
        { "version",    no_argument,       0, 'v' },
        { "help",       no_argument,       0, 'H' },
        { "type",       required_argument, 0, 't' },
        { "n",          required_argument, 0, 'n' },
        { "hue",        required_argument, 0, 'h' },
        { "divergence", required_argument, 0, 'd' },
        { "contrast",   required_argument, 0, 'c' },
        { "saturation", required_argument, 0, 's' },
        { "brightness", required_argument, 0, 'b' },
        { "warmth",     required_argument, 0, 'w' },
        { "rotations",  required_argument, 0, 'r' },
        { "gamma",      required_argument, 0, 'g' },
        { 0, 0, 0, 0 }
    };

    for (;;) {
        int c = getopt_long(argc, argv, "vHt:n:h:d:c:s:b:w:r:g:", options, NULL);
        if (c == -1)
            break;
        switch (c) {
        case 'v':
            print_version = true;
            break;
        case 'H':
            print_help = true;
            break;
        case 't':
            type = (strcmp(optarg, "brewer-sequential") == 0 ? brewer_sequential
                    : strcmp(optarg, "brewer-diverging") == 0 ? brewer_diverging
                    : strcmp(optarg, "brewer-qualitative") == 0 ? brewer_qualitative
                    : strcmp(optarg, "cubehelix") == 0 ? cubehelix
                    : -2);
            break;
        case 'n':
            n = atoi(optarg);
            break;
        case 'h':
            hue = atof(optarg) * M_PI / 180.0;
            break;
        case 'd':
            divergence = atof(optarg) * M_PI / 180.0;
            break;
        case 'c':
            contrast = atof(optarg);
            break;
        case 's':
            saturation = atof(optarg);
            break;
        case 'b':
            brightness = atof(optarg);
            break;
        case 'w':
            warmth = atof(optarg);
            break;
        case 'r':
            rotations = atof(optarg);
            break;
        case 'g':
            gamma = atof(optarg);
            break;
        default:
            return 1;
        }
    }

    if (print_version) {
        printf("gencolormap version 0.2\n"
                "Copyright (C) 2016 Computer Graphics Group, University of Siegen.\n"
                "Written by Martin Lambers <martin.lambers@uni-siegen.de>.\n"
                "This is free software under the terms of the MIT/Expat License.\n"
                "There is NO WARRANTY, to the extent permitted by law.\n");
        return 0;
    }

    if (print_help) {
        printf("Usage: %s\n"
                "  Common options, required for all types:\n"
                "    -n|--n=N                      Set number of colors in the map\n"
                "  Brewer-like color maps:\n"
                "    -t|--type=brewer-sequential   Generate a sequential color map\n"
                "    -t|--type=brewer-diverging    Generate a diverging color map\n"
                "    -t|--type=brewer-qualitative  Generate a qualitative color map\n"
                "    [-h|--hue=H]                  Set default hue in [0,360] degrees\n"
                "    [-c|--contrast=C]             Set contrast in [0,1]\n"
                "    [-s|--saturation=S]           Set saturation in [0,1]\n"
                "    [-b|--brightness=B]           Set brightness in [0,1]\n"
                "    [-w|--warmth=W]               Set warmth in [0,1] for seq. and div. maps\n"
                "    [-d|--divergence=D]           Set divergence for div. and qual. maps\n"
                "  CubeHelix color maps:\n"
                "    -t|--type=cubehelix           Generate a CubeHelix color map\n"
                "    [-r|--rotations=R]            Set number of rotations, in (-infty,infty)\n"
                "    [-s|--saturation=S]           Set saturation, in [0,1]\n"
                "    [-g|--gamma=G]                Set gamma correction, in (0,infty)\n"
                "Generates a color map and prints it to standard output as sRGB triplets.\n"
                "Report bugs to <martin.lambers@uni-siegen.de>.\n", argv[0]);
        return 0;
    }

    if (type < 0) {
        fprintf(stderr, "Invalid or missing option -t|--type.\n");
        return 1;
    }
    if (n < 2) {
        fprintf(stderr, "Invalid or missing option -n|--n.\n");
        return 1;
    }
    if (hue < 0.0f) {
        if (type == brewer_sequential)
            hue = ColorMap::BrewerSequentialDefaultHue;
        else if (type == brewer_diverging)
            hue = ColorMap::BrewerDivergingDefaultHue;
        else if (type == brewer_qualitative)
            hue = ColorMap::BrewerQualitativeDefaultHue;
        else if (type == cubehelix)
            hue = ColorMap::CubeHelixDefaultHue;
    }
    if (divergence < 0.0f) {
        if (type == brewer_diverging)
            divergence = ColorMap::BrewerDivergingDefaultDivergence;
        else if (type == brewer_qualitative)
            divergence = ColorMap::BrewerQualitativeDefaultDivergence;
    }
    if (contrast < 0.0f) {
        if (type == brewer_sequential)
            contrast = (n <= 9 ? ColorMap::BrewerSequentialDefaultContrastForSmallN(n)
                    : ColorMap::BrewerSequentialDefaultContrast);
        else if (type == brewer_diverging)
            contrast = (n <= 9 ? ColorMap::BrewerDivergingDefaultContrastForSmallN(n)
                    : ColorMap::BrewerDivergingDefaultContrast);
        else if (type == brewer_qualitative)
            contrast = ColorMap::BrewerQualitativeDefaultContrast;
    }
    if (saturation < 0.0f) {
        if (type == brewer_sequential)
            saturation = ColorMap::BrewerSequentialDefaultSaturation;
        else if (type == brewer_diverging)
            saturation = ColorMap::BrewerDivergingDefaultSaturation;
        else if (type == brewer_qualitative)
            saturation = ColorMap::BrewerQualitativeDefaultSaturation;
        else if (type == cubehelix)
            saturation = ColorMap::CubeHelixDefaultSaturation;
    }
    if (brightness < 0.0f) {
        if (type == brewer_sequential)
            brightness = ColorMap::BrewerSequentialDefaultBrightness;
        else if (type == brewer_diverging)
            brightness = ColorMap::BrewerDivergingDefaultBrightness;
        else if (type == brewer_qualitative)
            brightness = ColorMap::BrewerQualitativeDefaultBrightness;
    }
    if (warmth < 0.0f) {
        if (type == brewer_sequential)
            brightness = ColorMap::BrewerSequentialDefaultWarmth;
        else if (type == brewer_diverging)
            brightness = ColorMap::BrewerDivergingDefaultWarmth;
    }
    if (std::isnan(rotations)) {
        if (type == cubehelix)
            rotations = ColorMap::CubeHelixDefaultRotations;
    }
    if (gamma < 0.0f) {
        if (type == cubehelix)
            gamma = ColorMap::CubeHelixDefaultGamma;
    }

    std::vector<unsigned char> colormap(3 * n);
    switch (type) {
    case brewer_sequential:
        ColorMap::BrewerSequential(n, &(colormap[0]), hue, contrast, saturation, brightness, warmth);
        break;
    case brewer_diverging:
        ColorMap::BrewerDiverging(n, &(colormap[0]), hue, divergence, contrast, saturation, brightness, warmth);
        break;
    case brewer_qualitative:
        ColorMap::BrewerQualitative(n, &(colormap[0]), hue, divergence, contrast, saturation, brightness);
        break;
    case cubehelix:
        ColorMap::CubeHelix(n, &(colormap[0]), hue, rotations, saturation, gamma);
        break;
    }

    for (int i = 0; i < n; i++) {
        printf("%d, %d, %d\n", colormap[3 * i + 0], colormap[3 * i + 1], colormap[3 * i + 2]);
    }

    return 0;
}
