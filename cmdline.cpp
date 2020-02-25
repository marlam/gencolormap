/*
 * Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020
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
#include "export.hpp"

enum type {
    brewer_seq,
    brewer_div,
    brewer_qual,
    puseq_lightness,
    puseq_saturation,
    puseq_rainbow,
    puseq_blackbody,
    puseq_multihue,
    pudiv_lightness,
    pudiv_saturation,
    puqual_hue,
    cubehelix,
    moreland,
    mcnames
};

enum format {
    csv,
    json,
    ppm
};

int main(int argc, char* argv[])
{
    bool print_version = false;
    bool print_help = false;
    int format = csv;
    int type = brewer_seq;
    int n = 256;
    float hue = -1.0f;
    float divergence = -1.0f;
    float contrast = -1.0f;
    float saturation = -1.0f;
    float saturation_range = -1.0f;
    float brightness = -1.0f;
    float warmth = -1.0f;
    float lightness = -1.0f;
    float lightness_range = -1.0f;
    float rotations = NAN;
    float temperature = -1.0f;
    float temperature_range = -1.0f;
    std::vector<float> hue_values;
    std::vector<float> hue_positions;
    float gamma = -1.0f;
    bool have_color0 = false;
    unsigned char color0[3];
    bool have_color1 = false;
    unsigned char color1[3];
    float periods = NAN;
    struct option options[] = {
        { "version",           no_argument,       0, 'v' },
        { "help",              no_argument,       0, 'H' },
        { "format",            required_argument, 0, 'f' },
        { "type",              required_argument, 0, 't' },
        { "n",                 required_argument, 0, 'n' },
        { "hue",               required_argument, 0, 'h' },
        { "divergence",        required_argument, 0, 'd' },
        { "contrast",          required_argument, 0, 'c' },
        { "saturation",        required_argument, 0, 's' },
        { "saturation-range",  required_argument, 0, 'S' },
        { "brightness",        required_argument, 0, 'b' },
        { "warmth",            required_argument, 0, 'w' },
        { "lightness",         required_argument, 0, 'l' },
        { "lightness-range",   required_argument, 0, 'L' },
        { "rotations",         required_argument, 0, 'r' },
        { "temperature",       required_argument, 0, 'T' },
        { "temperature-range", required_argument, 0, 'R' },
        { "hue-values",        required_argument, 0, 'V' },
        { "hue-positions",     required_argument, 0, 'P' },
        { "gamma",             required_argument, 0, 'g' },
        { "color0",            required_argument, 0, 'A' },
        { "color1",            required_argument, 0, 'O' },
        { "periods",           required_argument, 0, 'p' },
        { 0, 0, 0, 0 }
    };

    for (;;) {
        int c = getopt_long(argc, argv, "vHf:t:n:h:d:c:s:S:b:w:l:L:r:T:R:V:P::g:A:O:p:", options, NULL);
        if (c == -1)
            break;
        switch (c) {
        case 'v':
            print_version = true;
            break;
        case 'H':
            print_help = true;
            break;
        case 'f':
            format = (strcmp(optarg, "csv") == 0 ? csv
                    : strcmp(optarg, "json") == 0 ? json
                    : strcmp(optarg, "ppm") == 0 ? ppm
                    : -1);
            break;
        case 't':
            type = (strcmp(optarg, "brewer-sequential") == 0 ? brewer_seq
                    : strcmp(optarg, "brewer-diverging") == 0 ? brewer_div
                    : strcmp(optarg, "brewer-qualitative") == 0 ? brewer_qual
                    : strcmp(optarg, "pusequential-lightness") == 0 ? puseq_lightness
                    : strcmp(optarg, "pusequential-saturation") == 0 ? puseq_saturation
                    : strcmp(optarg, "pusequential-rainbow") == 0 ? puseq_rainbow
                    : strcmp(optarg, "pusequential-blackbody") == 0 ? puseq_blackbody
                    : strcmp(optarg, "pusequential-multihue") == 0 ? puseq_multihue
                    : strcmp(optarg, "pudiverging-lightness") == 0 ? pudiv_lightness
                    : strcmp(optarg, "pudiverging-saturation") == 0 ? pudiv_saturation
                    : strcmp(optarg, "puqualitative-hue") == 0 ? puqual_hue
                    : strcmp(optarg, "cubehelix") == 0 ? cubehelix
                    : strcmp(optarg, "moreland") == 0 ? moreland
                    : strcmp(optarg, "mcnames") == 0 ? mcnames
                    : -1);
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
        case 'S':
            saturation_range = atof(optarg);
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
        case 'L':
            lightness_range = atof(optarg);
            break;
        case 'r':
            rotations = atof(optarg);
            break;
        case 'T':
            temperature = atof(optarg);
            break;
        case 'R':
            temperature_range = atof(optarg);
            break;
        case 'V':
            for (;;) {
                hue_values.push_back(atof(optarg) * M_PI / 180.0);
                optarg = strchr(optarg, ',');
                if (!optarg)
                    break;
                optarg++;
            }
            break;
        case 'P':
            for (;;) {
                hue_positions.push_back(atof(optarg));
                optarg = strchr(optarg, ',');
                if (!optarg)
                    break;
                optarg++;
            }
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
        printf("gencolormap version 2.0\n"
                "https://marlam.de/gencolormap\n"
                "Copyright (C) 2020 Computer Graphics Group, University of Siegen.\n"
                "Written by Martin Lambers <martin.lambers@uni-siegen.de>.\n"
                "This is free software under the terms of the MIT/Expat License.\n"
                "There is NO WARRANTY, to the extent permitted by law.\n");
        return 0;
    }

    if (print_help) {
        printf("Usage: %s [option...]\n"
                "Generates a color map and prints it to standard output.\n"
                "Prints the number of colors that had to be clipped to standard error.\n"
                "Common options:\n"
                "  [-f|--format=csv|json|ppm]          Set output format\n"
                "  [-n|--n=N]                          Set number of colors in the map\n"
                "Brewer-like color maps:\n"
                "  [-t|--type=brewer-sequential]       Generate a sequential color map\n"
                "  [-t|--type=brewer-diverging]        Generate a diverging color map\n"
                "  [-t|--type=brewer-qualitative]      Generate a qualitative color map\n"
                "  [-h|--hue=H]                        Set default hue in [0,360] degrees\n"
                "  [-c|--contrast=C]                   Set contrast in [0,1]\n"
                "  [-s|--saturation=S]                 Set saturation in [0,1]\n"
                "  [-b|--brightness=B]                 Set brightness in [0,1]\n"
                "  [-w|--warmth=W]                     Set warmth in [0,1] for seq. and div. maps\n"
                "  [-d|--divergence=D]                 Set diverg. in deg for div. and qual. maps\n"
                "Perceptually uniform color maps:\n"
                "  [-t|--type=pusequential-lightness]  Sequential map, varying lightness\n"
                "  [-t|--type=pusequential-saturation] Sequential map, varying saturation\n"
                "  [-t|--type=pusequential-rainbow]    Sequential map, varying hue (rainbow)\n"
                "  [-t|--type=pusequential-blackbody]  Sequential map, varying hue (black body)\n"
                "  [-t|--type=pusequential-multihue]   Sequential map, varying hue (custom)\n"
                "  [-t|--type=pudiverging-lightness]   Diverging map, varying lightness\n"
                "  [-t|--type=pudiverging-saturation]  Diverging map, varying saturation\n"
                "  [-t|--type=puqualitative-hue]       Qualitative map, evenly distributed hue\n"
                "  [-l|--lightness=L]                  Set lightness in [0,1]\n"
                "  [-L|--lightness-range=LR]           Set lightness range in [0.7,1]\n"
                "  [-s|--saturation=S]                 Set saturation in [0,1]\n"
                "  [-S|--saturation-range=SR]          Set saturation range in [0.7,1]\n"
                "  [-h|--hue=H]                        Set default hue in [0,360] degrees\n"
                "  [-d|--divergence=D]                 Set diverg. in deg for div. and qual. maps\n"
                "  [-r|--rotations=R]                  Set number of rotations for rainbow maps\n"
                "  [-T|--temperature=T]                Set start temp. in K for black body maps\n"
                "  [-R|--temperature-range=TR]         Set range for temperature in K\n"
                "  [-V|--hue-values=H0,H1,...]         Set hue values in [0,360] for multi-hue maps\n"
                "  [-P|--hue-positions=P0,P1,...]      Set hue positions in [0,1] for multi-hue maps\n"
                "CubeHelix color maps:\n"
                "  [-t|--type=cubehelix]               Generate a CubeHelix color map\n"
                "  [-h|--hue=H]                        Set start hue in [0,180] degrees\n"
                "  [-r|--rotations=R]                  Set number of rotations, in (-infty,infty)\n"
                "  [-s|--saturation=S]                 Set saturation, in [0,1]\n"
                "  [-g|--gamma=G]                      Set gamma correction, in (0,infty)\n"
                "Moreland diverging color maps:\n"
                "  [-t|--type=moreland]                Generate a Moreland diverging color map\n"
                "  [-A|--color0=sr,sg,sb]              Set the first color as sRGB in [0,255]\n"
                "  [-O|--color1=sr,sg,sb]              Set the last color as sRGB in [0,255]\n"
                "McNames sequential color maps:\n"
                "  [-t|--type=mcnames]                 Generate a McNames sequential color map\n"
                "  [-p|--periods=P]                    Set the number of periods in (0, infty)\n"
                "Defaults: format=csv, n=256, type=brewer-sequential\n"
                "https://marlam.de/gencolormap\n", argv[0]);
        return 0;
    }

    if (format < 0) {
        fprintf(stderr, "Invalid argument for option -f|--format.\n");
        return 1;
    }
    if (n < 2) {
        fprintf(stderr, "Invalid argument for option -n|--n.\n");
        return 1;
    }
    if (type < 0) {
        fprintf(stderr, "Invalid argument for option -t|--type.\n");
        return 1;
    }
    if (hue < 0.0f) {
        if (type == brewer_seq)
            hue = ColorMap::BrewerSequentialDefaultHue;
        else if (type == brewer_div)
            hue = ColorMap::BrewerDivergingDefaultHue;
        else if (type == brewer_qual)
            hue = ColorMap::BrewerQualitativeDefaultHue;
        else if (type == puseq_lightness)
            hue = ColorMap::PUSequentialLightnessDefaultHue;
        else if (type == puseq_saturation)
            hue = ColorMap::PUSequentialSaturationDefaultHue;
        else if (type == puseq_rainbow)
            hue = ColorMap::PUSequentialRainbowDefaultHue;
        else if (type == pudiv_lightness)
            hue = ColorMap::PUDivergingLightnessDefaultHue;
        else if (type == pudiv_saturation)
            hue = ColorMap::PUDivergingSaturationDefaultHue;
        else if (type == puqual_hue)
            hue = ColorMap::PUQualitativeHueDefaultHue;
        else if (type == cubehelix)
            hue = ColorMap::CubeHelixDefaultHue;
    }
    if (divergence < 0.0f) {
        if (type == brewer_div)
            divergence = ColorMap::BrewerDivergingDefaultDivergence;
        else if (type == brewer_qual)
            divergence = ColorMap::BrewerQualitativeDefaultDivergence;
        else if (type == pudiv_lightness)
            divergence = ColorMap::PUDivergingLightnessDefaultDivergence;
        else if (type == pudiv_saturation)
            divergence = ColorMap::PUDivergingSaturationDefaultDivergence;
        else if (type == puqual_hue)
            divergence = ColorMap::PUQualitativeHueDefaultDivergence;
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
        else if (type == puseq_lightness)
            saturation = ColorMap::PUSequentialLightnessDefaultSaturation;
        else if (type == puseq_saturation)
            saturation = ColorMap::PUSequentialSaturationDefaultSaturation;
        else if (type == puseq_rainbow)
            saturation = ColorMap::PUSequentialRainbowDefaultSaturation;
        else if (type == puseq_blackbody)
            saturation = ColorMap::PUSequentialBlackBodyDefaultSaturation;
        else if (type == puseq_multihue)
            saturation = ColorMap::PUSequentialMultiHueDefaultSaturation;
        else if (type == pudiv_lightness)
            saturation = ColorMap::PUDivergingLightnessDefaultSaturation;
        else if (type == pudiv_saturation)
            saturation = ColorMap::PUDivergingSaturationDefaultSaturation;
        else if (type == puqual_hue)
            saturation = ColorMap::PUQualitativeHueDefaultSaturation;
        else if (type == cubehelix)
            saturation = ColorMap::CubeHelixDefaultSaturation;
    }
    if (saturation_range < 0.0f) {
        if (type == puseq_lightness)
            saturation_range = ColorMap::PUSequentialLightnessDefaultSaturationRange;
        else if (type == puseq_saturation)
            saturation_range = ColorMap::PUSequentialSaturationDefaultSaturationRange;
        else if (type == puseq_rainbow)
            saturation_range = ColorMap::PUSequentialRainbowDefaultSaturationRange;
        else if (type == puseq_blackbody)
            saturation_range = ColorMap::PUSequentialBlackBodyDefaultSaturationRange;
        else if (type == puseq_multihue)
            saturation_range = ColorMap::PUSequentialMultiHueDefaultSaturationRange;
        else if (type == pudiv_lightness)
            saturation_range = ColorMap::PUDivergingLightnessDefaultSaturationRange;
        else if (type == pudiv_saturation)
            saturation_range = ColorMap::PUDivergingSaturationDefaultSaturationRange;
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
        if (type == puseq_saturation)
            lightness = ColorMap::PUSequentialSaturationDefaultLightness;
        else if (type == pudiv_saturation)
            lightness = ColorMap::PUDivergingSaturationDefaultLightness;
        else if (type == puqual_hue)
            lightness = ColorMap::PUQualitativeHueDefaultLightness;
    }
    if (lightness_range < 0.0f) {
        if (type == puseq_lightness)
            lightness_range = ColorMap::PUSequentialLightnessDefaultLightnessRange;
        else if (type == puseq_rainbow)
            lightness_range = ColorMap::PUSequentialRainbowDefaultLightnessRange;
        else if (type == puseq_blackbody)
            lightness_range = ColorMap::PUSequentialBlackBodyDefaultLightnessRange;
        else if (type == puseq_multihue)
            lightness_range = ColorMap::PUSequentialMultiHueDefaultLightnessRange;
        else if (type == pudiv_lightness)
            lightness_range = ColorMap::PUDivergingLightnessDefaultLightnessRange;
    }
    if (std::isnan(rotations)) {
        if (type == puseq_rainbow)
            rotations = ColorMap::PUSequentialRainbowDefaultRotations;
        else if (type == cubehelix)
            rotations = ColorMap::CubeHelixDefaultRotations;
    }
    if (temperature < 0.0f) {
        if (type == puseq_blackbody)
            temperature = ColorMap::PUSequentialBlackBodyDefaultTemperature;
    }
    if (temperature_range < 0.0f) {
        if (type == puseq_blackbody)
            temperature_range = ColorMap::PUSequentialBlackBodyDefaultTemperatureRange;
    }
    if (gamma < 0.0f) {
        if (type == cubehelix)
            gamma = ColorMap::CubeHelixDefaultGamma;
    }
    if (hue_values.size() == 0) {
        if (type == puseq_multihue) {
            hue_values.resize(ColorMap::PUSequentialMultiHueDefaultHues);
            for (size_t i = 0; i < hue_values.size(); i++)
                hue_values[i] = ColorMap::PUSequentialMultiHueDefaultHueValues[i];
        }
    }
    if (hue_positions.size() == 0) {
        if (type == puseq_multihue) {
            hue_positions.resize(ColorMap::PUSequentialMultiHueDefaultHues);
            for (size_t i = 0; i < hue_positions.size(); i++)
                hue_positions[i] = ColorMap::PUSequentialMultiHueDefaultHuePositions[i];
        }
    }
    if (hue_values.size() != hue_positions.size()) {
        fprintf(stderr, "Number of hue values and positions do not match.\n");
        return 1;
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
    case puseq_lightness:
        clipped = ColorMap::PUSequentialLightness(n, &(colormap[0]), lightness_range, saturation_range, saturation, hue);
        break;
    case puseq_saturation:
        clipped = ColorMap::PUSequentialSaturation(n, &(colormap[0]), saturation_range, lightness, saturation, hue);
        break;
    case puseq_rainbow:
        clipped = ColorMap::PUSequentialRainbow(n, &(colormap[0]), lightness_range, saturation_range, hue, rotations, saturation);
        break;
    case puseq_blackbody:
        clipped = ColorMap::PUSequentialBlackBody(n, &(colormap[0]), temperature, temperature_range, lightness_range, saturation_range, saturation);
        break;
    case puseq_multihue:
        clipped = ColorMap::PUSequentialMultiHue(n, &(colormap[0]), lightness_range, saturation_range, saturation,
                hue_values.size(), hue_values.data(), hue_positions.data());
        break;
    case pudiv_lightness:
        clipped = ColorMap::PUDivergingLightness(n, &(colormap[0]), lightness_range, saturation_range, saturation, hue, divergence);
        break;
    case pudiv_saturation:
        clipped = ColorMap::PUDivergingSaturation(n, &(colormap[0]), saturation_range, lightness, saturation, hue, divergence);
        break;
    case puqual_hue:
        clipped = ColorMap::PUQualitativeHue(n, &(colormap[0]), hue, divergence, lightness, saturation);
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

    std::string output;
    if (format == csv) {
        output = ColorMap::ToCSV(n, colormap.data());
    } else if (format == json) {
        output = ColorMap::ToJSON(n, colormap.data());
    } else {
        output = ColorMap::ToPPM(n, colormap.data());
    }
    fputs(output.c_str(), stdout);
    fprintf(stderr, "%d color(s) were clipped\n", clipped);

    return 0;
}
