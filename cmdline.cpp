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
    brewer_seq,
    brewer_div,
    brewer_qual,
    plseq_lightness,
    plseq_saturation,
    plseq_rainbow,
    plseq_blackbody,
    pldiv_lightness,
    pldiv_saturation,
    plqual_hue,
    cubehelix,
    moreland,
    mcnames
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
    float lightness = -1.0f;
    float rotations = NAN;
    float temperature = -1.0f;
    float range = -1.0f;
    float gamma = -1.0f;
    bool have_color0 = false;
    unsigned char color0[3];
    bool have_color1 = false;
    unsigned char color1[3];
    float periods = NAN;
    struct option options[] = {
        { "version",     no_argument,       0, 'v' },
        { "help",        no_argument,       0, 'H' },
        { "type",        required_argument, 0, 't' },
        { "n",           required_argument, 0, 'n' },
        { "hue",         required_argument, 0, 'h' },
        { "divergence",  required_argument, 0, 'd' },
        { "contrast",    required_argument, 0, 'c' },
        { "saturation",  required_argument, 0, 's' },
        { "brightness",  required_argument, 0, 'b' },
        { "warmth",      required_argument, 0, 'w' },
        { "lightness",   required_argument, 0, 'l' },
        { "rotations",   required_argument, 0, 'r' },
        { "temperature", required_argument, 0, 'T' },
        { "range",       required_argument, 0, 'R' },
        { "gamma",       required_argument, 0, 'g' },
        { "color0",      required_argument, 0, 'A' },
        { "color1",      required_argument, 0, 'O' },
        { "periods",     required_argument, 0, 'p' },
        { 0, 0, 0, 0 }
    };

    for (;;) {
        int c = getopt_long(argc, argv, "vHt:n:h:d:c:s:b:w:l:T:R:r:g:A:O:p:", options, NULL);
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
            type = (strcmp(optarg, "brewer-sequential") == 0 ? brewer_seq
                    : strcmp(optarg, "brewer-diverging") == 0 ? brewer_div
                    : strcmp(optarg, "brewer-qualitative") == 0 ? brewer_qual
                    : strcmp(optarg, "plsequential-lightness") == 0 ? plseq_lightness
                    : strcmp(optarg, "plsequential-saturation") == 0 ? plseq_saturation
                    : strcmp(optarg, "plsequential-rainbow") == 0 ? plseq_rainbow
                    : strcmp(optarg, "plsequential-blackbody") == 0 ? plseq_blackbody
                    : strcmp(optarg, "pldiverging-lightness") == 0 ? pldiv_lightness
                    : strcmp(optarg, "pldiverging-saturation") == 0 ? pldiv_saturation
                    : strcmp(optarg, "plqualitative-hue") == 0 ? plqual_hue
                    : strcmp(optarg, "cubehelix") == 0 ? cubehelix
                    : strcmp(optarg, "moreland") == 0 ? moreland
                    : strcmp(optarg, "mcnames") == 0 ? mcnames
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
        case 'l':
            lightness = atof(optarg);
            break;
        case 'r':
            rotations = atof(optarg);
            break;
        case 'T':
            temperature = atof(optarg);
            break;
        case 'R':
            range = atof(optarg);
            break;
        case 'g':
            gamma = atof(optarg);
            break;
        case 'A':
            std::sscanf(optarg, "%hhu,%hhu,%hhu", color0 + 0, color0 + 1, color0 + 2);
            have_color0 = true;
            break;
        case 'O':
            std::sscanf(optarg, "%hhu,%hhu,%hhu", color1 + 0, color1 + 1, color1 + 2);
            have_color1 = true;
            break;
        case 'p':
            periods = atof(optarg);
            break;
        default:
            return 1;
        }
    }

    if (print_version) {
        printf("gencolormap version 0.3\n"
                "Copyright (C) 2016 Computer Graphics Group, University of Siegen.\n"
                "Written by Martin Lambers <martin.lambers@uni-siegen.de>.\n"
                "This is free software under the terms of the MIT/Expat License.\n"
                "There is NO WARRANTY, to the extent permitted by law.\n");
        return 0;
    }

    if (print_help) {
        printf("Usage: %s\n"
                "  Common options, required for all types:\n"
                "    -n|--n=N                         Set number of colors in the map\n"
                "  Brewer-like color maps:\n"
                "    -t|--type=brewer-sequential       Generate a sequential color map\n"
                "    -t|--type=brewer-diverging        Generate a diverging color map\n"
                "    -t|--type=brewer-qualitative      Generate a qualitative color map\n"
                "    [-h|--hue=H]                      Set default hue in [0,360] degrees\n"
                "    [-c|--contrast=C]                 Set contrast in [0,1]\n"
                "    [-s|--saturation=S]               Set saturation in [0,1]\n"
                "    [-b|--brightness=B]               Set brightness in [0,1]\n"
                "    [-w|--warmth=W]                   Set warmth in [0,1] for seq. and div. maps\n"
                "    [-d|--divergence=D]               Set divergence in deg. for div. and qual. maps\n"
                "  Perceptually linear color maps:\n"
                "    -t|--type=plsequential-lightness  Sequential map with varying lightness\n"
                "    -t|--type=plsequential-saturation Sequential map with varying saturation\n"
                "    -t|--type=plsequential-rainbow    Sequential map with varying hue (rainbow)\n"
                "    -t|--type=plsequential-blackbody  Sequential map with varying hue (black body)\n"
                "    -t|--type=pldiverging-lightness   Diverging map with varying lightness\n"
                "    -t|--type=pldiverging-saturation  Diverging map with varying saturation\n"
                "    -t|--type=plqualitative-hue       Qualitative map with evenly distributed hue\n"
                "    [-l|--lightness=L]                Set lightness in [0,1]\n"
                "    [-s|--saturation=S]               Set saturation in [0,1]\n"
                "    [-h|--hue=H]                      Set default hue in [0,360] degrees\n"
                "    [-d|--divergence=D]               Set divergence in deg. for div. and qual. maps\n"
                "    [-r|--rotations=R]                Set number of rotations for rainbow maps\n"
                "    [-T|--temperature=T]              Set start temp. in K for blackbody maps\n"
                "    [-R|--range=R]                    Set temp. range in K for blackbody maps\n"
                "  CubeHelix color maps:\n"
                "    -t|--type=cubehelix               Generate a CubeHelix color map\n"
                "    [-h|--hue=H]                      Set start hue in [0,180] degrees\n"
                "    [-r|--rotations=R]                Set number of rotations, in (-infty,infty)\n"
                "    [-s|--saturation=S]               Set saturation, in [0,1]\n"
                "    [-g|--gamma=G]                    Set gamma correction, in (0,infty)\n"
                "  Moreland diverging color maps:\n"
                "    -t|--type=moreland                Generate a Moreland diverging color map\n"
                "    [-A|--color0=sr,sg,sb             Set the first color as sRGB values in [0,255]\n"
                "    [-O|--color1=sr,sg,sb             Set the last color as sRGB values in [0,255]\n"
                "  McNames sequential color maps:\n"
                "    -t|--type=mcnames                 Generate a McNames sequential color map\n"
                "    [-p|--periods=P]                  Set the number of periods in (0, infty)\n"
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
        if (type == brewer_seq)
            hue = ColorMap::BrewerSequentialDefaultHue;
        else if (type == brewer_div)
            hue = ColorMap::BrewerDivergingDefaultHue;
        else if (type == brewer_qual)
            hue = ColorMap::BrewerQualitativeDefaultHue;
        else if (type == plseq_lightness)
            hue = ColorMap::PLSequentialLightnessDefaultHue;
        else if (type == plseq_saturation)
            hue = ColorMap::PLSequentialSaturationDefaultHue;
        else if (type == plseq_rainbow)
            hue = ColorMap::PLSequentialRainbowDefaultHue;
        else if (type == pldiv_lightness)
            hue = ColorMap::PLDivergingLightnessDefaultHue;
        else if (type == pldiv_saturation)
            hue = ColorMap::PLDivergingSaturationDefaultHue;
        else if (type == plqual_hue)
            hue = ColorMap::PLQualitativeHueDefaultHue;
        else if (type == cubehelix)
            hue = ColorMap::CubeHelixDefaultHue;
    }
    if (divergence < 0.0f) {
        if (type == brewer_div)
            divergence = ColorMap::BrewerDivergingDefaultDivergence;
        else if (type == brewer_qual)
            divergence = ColorMap::BrewerQualitativeDefaultDivergence;
        else if (type == pldiv_lightness)
            divergence = ColorMap::PLDivergingLightnessDefaultDivergence;
        else if (type == pldiv_saturation)
            divergence = ColorMap::PLDivergingSaturationDefaultDivergence;
        else if (type == plqual_hue)
            divergence = ColorMap::PLQualitativeHueDefaultDivergence;
    }
    if (contrast < 0.0f) {
        if (type == brewer_seq)
            contrast = (n <= 9 ? ColorMap::BrewerSequentialDefaultContrastForSmallN(n)
                    : ColorMap::BrewerSequentialDefaultContrast);
        else if (type == brewer_div)
            contrast = (n <= 9 ? ColorMap::BrewerDivergingDefaultContrastForSmallN(n)
                    : ColorMap::BrewerDivergingDefaultContrast);
        else if (type == brewer_qual)
            contrast = ColorMap::BrewerQualitativeDefaultContrast;
    }
    if (saturation < 0.0f) {
        if (type == brewer_seq)
            saturation = ColorMap::BrewerSequentialDefaultSaturation;
        else if (type == brewer_div)
            saturation = ColorMap::BrewerDivergingDefaultSaturation;
        else if (type == brewer_qual)
            saturation = ColorMap::BrewerQualitativeDefaultSaturation;
        else if (type == plseq_lightness)
            saturation = ColorMap::PLSequentialLightnessDefaultSaturation;
        else if (type == plseq_saturation)
            saturation = ColorMap::PLSequentialSaturationDefaultSaturation;
        else if (type == plseq_rainbow)
            saturation = ColorMap::PLSequentialRainbowDefaultSaturation;
        else if (type == plseq_blackbody)
            saturation = ColorMap::PLSequentialBlackBodyDefaultSaturation;
        else if (type == pldiv_lightness)
            saturation = ColorMap::PLDivergingLightnessDefaultSaturation;
        else if (type == pldiv_saturation)
            saturation = ColorMap::PLDivergingSaturationDefaultSaturation;
        else if (type == plqual_hue)
            saturation = ColorMap::PLQualitativeHueDefaultSaturation;
        else if (type == cubehelix)
            saturation = ColorMap::CubeHelixDefaultSaturation;
    }
    if (brightness < 0.0f) {
        if (type == brewer_seq)
            brightness = ColorMap::BrewerSequentialDefaultBrightness;
        else if (type == brewer_div)
            brightness = ColorMap::BrewerDivergingDefaultBrightness;
        else if (type == brewer_qual)
            brightness = ColorMap::BrewerQualitativeDefaultBrightness;
    }
    if (warmth < 0.0f) {
        if (type == brewer_seq)
            warmth = ColorMap::BrewerSequentialDefaultWarmth;
        else if (type == brewer_div)
            warmth = ColorMap::BrewerDivergingDefaultWarmth;
    }
    if (lightness < 0.0f) {
        if (type == plseq_saturation)
            lightness = ColorMap::PLSequentialSaturationDefaultLightness;
        else if (type == pldiv_lightness)
            lightness = ColorMap::PLDivergingLightnessDefaultLightness;
        else if (type == pldiv_saturation)
            lightness = ColorMap::PLDivergingSaturationDefaultLightness;
        else if (type == plqual_hue)
            lightness = ColorMap::PLQualitativeHueDefaultLightness;
    }
    if (std::isnan(rotations)) {
        if (type == plseq_rainbow)
            rotations = ColorMap::PLSequentialRainbowDefaultRotations;
        else if (type == cubehelix)
            rotations = ColorMap::CubeHelixDefaultRotations;
    }
    if (temperature < 0.0f) {
        if (type == plseq_blackbody)
            temperature = ColorMap::PLSequentialBlackBodyDefaultTemperature;
    }
    if (range < 0.0f) {
        if (type == plseq_blackbody)
            range = ColorMap::PLSequentialBlackBodyDefaultRange;
    }
    if (gamma < 0.0f) {
        if (type == cubehelix)
            gamma = ColorMap::CubeHelixDefaultGamma;
    }
    if (!have_color0) {
        if (type == moreland) {
            color0[0] = ColorMap::MorelandDefaultR0;
            color0[1] = ColorMap::MorelandDefaultG0;
            color0[2] = ColorMap::MorelandDefaultB0;
        }
    }
    if (!have_color1) {
        if (type == moreland) {
            color1[0] = ColorMap::MorelandDefaultR1;
            color1[1] = ColorMap::MorelandDefaultG1;
            color1[2] = ColorMap::MorelandDefaultB1;
        }
    }
    if (std::isnan(periods)) {
        if (type == mcnames)
            periods = ColorMap::McNamesDefaultPeriods;
    }

    std::vector<unsigned char> colormap(3 * n);
    int clipped;
    switch (type) {
    case brewer_seq:
        clipped = ColorMap::BrewerSequential(n, &(colormap[0]), hue, contrast, saturation, brightness, warmth);
        break;
    case brewer_div:
        clipped = ColorMap::BrewerDiverging(n, &(colormap[0]), hue, divergence, contrast, saturation, brightness, warmth);
        break;
    case brewer_qual:
        clipped = ColorMap::BrewerQualitative(n, &(colormap[0]), hue, divergence, contrast, saturation, brightness);
        break;
    case plseq_lightness:
        clipped = ColorMap::PLSequentialLightness(n, &(colormap[0]), saturation, hue);
        break;
    case plseq_saturation:
        clipped = ColorMap::PLSequentialSaturation(n, &(colormap[0]), lightness, saturation, hue);
        break;
    case plseq_rainbow:
        clipped = ColorMap::PLSequentialRainbow(n, &(colormap[0]), hue, rotations, saturation);
        break;
    case plseq_blackbody:
        clipped = ColorMap::PLSequentialBlackBody(n, &(colormap[0]), temperature, range, saturation);
        break;
    case pldiv_lightness:
        clipped = ColorMap::PLDivergingLightness(n, &(colormap[0]), lightness, saturation, hue, divergence);
        break;
    case pldiv_saturation:
        clipped = ColorMap::PLDivergingSaturation(n, &(colormap[0]), lightness, saturation, hue, divergence);
        break;
    case plqual_hue:
        clipped = ColorMap::PLQualitativeHue(n, &(colormap[0]), hue, divergence, lightness, saturation);
        break;
    case cubehelix:
        clipped = ColorMap::CubeHelix(n, &(colormap[0]), hue, rotations, saturation, gamma);
        break;
    case moreland:
        clipped = ColorMap::Moreland(n, &(colormap[0]),
                color0[0], color0[1], color0[2],
                color1[0], color1[1], color1[2]);
        break;
    case mcnames:
        clipped = ColorMap::McNames(n, &(colormap[0]), periods);
        break;
    }

    for (int i = 0; i < n; i++) {
        printf("%d, %d, %d\n", colormap[3 * i + 0], colormap[3 * i + 1], colormap[3 * i + 2]);
    }
    fprintf(stderr, "%d color(s) were clipped\n", clipped);

    return 0;
}
