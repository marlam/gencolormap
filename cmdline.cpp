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

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <getopt.h>
extern char *optarg;
extern int optind;

#include "colormap.hpp"


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
        { 0, 0, 0, 0 }
    };
    int retval = 0;

    for (;;) {
        int c = getopt_long(argc, argv, "vHt:n:h:d:c:s:b:w:", options, NULL);
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
            type = (strcmp(optarg, "sequential") == 0 ? 0
                    : strcmp(optarg, "diverging") == 0 ? 1
                    : strcmp(optarg, "qualitative") == 0 ? 2
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
        default:
            retval = 1;
            break;
        }
    }
    if (retval != 0)
        return retval;

    if (print_version) {
        printf("gencolormap version 0.1\n"
                "Copyright (C) 2015 Computer Graphics Group, University of Siegen.\n"
                "Written by Martin Lambers <martin.lambers@uni-siegen.de>.\n"
                "This is free software under the terms of the MIT/Expat License.\n"
                "There is NO WARRANTY, to the extent permitted by law.\n");
        return retval;
    }

    if (type < 0) {
        fprintf(stderr, "Invalid or missing option -t|--type.\n");
        print_help = true;
        retval = 1;
    }
    if (n < 2) {
        fprintf(stderr, "Invalid or missing option -n|--n.\n");
        print_help = true;
        retval = 1;
    }
    if (hue < 0.0f) {
        fprintf(stderr, "Invalid or missing option -h|--hue.\n");
        print_help = true;
        retval = 1;
    }
    if (divergence < 0.0f) {
        if (type <= 1)
            divergence = 2.0f / 3.0f * static_cast<float>(M_PI);
        else
            divergence = ColorMap::DefaultQualitativeDivergence;
    }
    if (contrast < 0.0f) {
        if (type <= 1)
            if (n > 0 && n < 9)
                contrast = ColorMap::DefaultContrastForSmallN(n);
            else
                contrast = ColorMap::DefaultContrast;
        else
            contrast = ColorMap::DefaultQualitativeContrast;
    }
    if (saturation < 0.0f) {
        if (type <= 1)
            saturation = ColorMap::DefaultSaturation;
        else
            saturation = ColorMap::DefaultQualitativeSaturation;
    }
    if (brightness < 0.0f) {
        if (type <= 1)
            brightness = ColorMap::DefaultBrightness;
        else
            brightness = ColorMap::DefaultQualitativeBrightness;
    }
    if (warmth < 0.0f) {
        warmth = ColorMap::DefaultWarmth;
    }

    if (print_help) {
        printf("Usage: %s\n"
                "  -t|--type=sequential   Generate a sequential color map\n"
                "  -t|--type=diverging    Generate a diverging color map\n"
                "  -t|--type=qualitative  Generate a sequential color map\n"
                "  -n|--n=N               Set number of colors in the map\n"
                "  -h|--hue=H             Set hue in [0,360] degrees\n"
                "  [-d|--divergence=D]    Set divergence for div. and qual. maps\n"
                "  [-c|--contrast=C]      Set contrast in [0,1]\n"
                "  [-s|--saturation=S]    Set saturation in [0,1]\n"
                "  [-b|--brightness=B]    Set brightness in [0,1]\n"
                "  [-w|--warmth=W]        Set color warmth in [0,1] for seq. and div. maps\n"
                "Generates a color map and prints it to standard output as sRGB triplets.\n"
                "Report bugs to <martin.lambers@uni-siegen.de>.\n", argv[0]);
        return retval;
    }

    std::vector<unsigned char> colormap(3 * n);
    if (type == 0) {
        ColorMap::Sequential(n, &(colormap[0]), hue,
                contrast, saturation, brightness, warmth);
    } else if (type == 1) {
        ColorMap::Diverging(n, &(colormap[0]), hue, divergence,
                contrast, saturation, brightness, warmth);
    } else {
        ColorMap::Qualitative(n, &(colormap[0]), hue, divergence,
                contrast, saturation, brightness);
    }

    for (int i = 0; i < n; i++) {
        printf("%d, %d, %d\n", colormap[3 * i + 0], colormap[3 * i + 1], colormap[3 * i + 2]);
    }

    return retval;
}
