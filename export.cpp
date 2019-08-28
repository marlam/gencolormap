/*
 * Copyright (C) 2019
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

#include "export.hpp"

namespace ColorMap {

std::string ToCSV(int n, const unsigned char* srgb_colormap)
{
    std::string s;
    for (int i = 0; i < n; i++) {
        s +=  std::to_string(srgb_colormap[3 * i + 0]) + ", "
            + std::to_string(srgb_colormap[3 * i + 1]) + ", "
            + std::to_string(srgb_colormap[3 * i + 2]) + '\n';
    }
    return s;
}

std::string ToJSON(int n, const unsigned char* srgb_colormap)
{
    std::string s =
        "[\n"
        "{\n"
        "\"ColorSpace\" : \"RGB\",\n"
        "\"Name\" : \"GenColorMapGenerated\",\n"
        "\"NanColor\" : [ -1, -1, -1 ],\n"
        "\"RGBPoints\" : [\n";
    for (int i = 0; i < n; i++) {
        s +=  std::to_string(i / float(n - 1)) + ", "
            + std::to_string(srgb_colormap[3 * i + 0] / 255.0f) + ", "
            + std::to_string(srgb_colormap[3 * i + 1] / 255.0f) + ", "
            + std::to_string(srgb_colormap[3 * i + 2] / 255.0f) + (i == n - 1 ? "\n" : ",\n");
    }
    s += "]\n}\n]\n";
    return s;
}

std::string ToPPM(int n, const unsigned char* srgb_colormap)
{
    std::string s = "P3\n"; // magic number for plain PPM
    s += std::to_string(n) + " 1\n"; // width and height
    s += "255\n"; // max val
    for (int i = 0; i < n; i++) {
        s +=  std::to_string(srgb_colormap[3 * i + 0]) + ' '
            + std::to_string(srgb_colormap[3 * i + 1]) + ' '
            + std::to_string(srgb_colormap[3 * i + 2]) + '\n';
    }
    return s;
}

}
