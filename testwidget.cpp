/*
 * Copyright (C)  2016 Computer Graphics Group, University of Siegen
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

#include <cmath>

#include <QGuiApplication>

#include "testwidget.hpp"


/* Apply color map to test image. See
 * http://peterkovesi.com/projects/colourmaps/colourmaptestimage.html */

static const int W = 512;
static const int H = 128;
static const float twopi = 2.0 * M_PI;

ColorMapTestWidget::ColorMapTestWidget() : QLabel()
{
    setMinimumSize(W * qApp->devicePixelRatio(), H * qApp->devicePixelRatio());
    QVector<QColor> initial_colormap;
    initial_colormap.append(QColor(0, 0, 0));
    update(initial_colormap);
}

ColorMapTestWidget::~ColorMapTestWidget()
{
}

void ColorMapTestWidget::update(const QVector<QColor>& colormap)
{
    QImage img(W * qApp->devicePixelRatio(), H * qApp->devicePixelRatio(), QImage::Format_RGB32);
    for (int y = 0; y < img.height(); y++) {
        QRgb* scanline = reinterpret_cast<QRgb*>(img.scanLine(y));
        float v = 1.0f - (y / (img.height() - 1.0f));
        for (int x = 0; x < img.width(); x++) {
            float u = x / (img.width() - 1.0f);
            // Test image formula
            float ramp = u;
            float modulation = 0.05f * std::sin(W / 8 * twopi * u);
            float value = ramp + v * v * modulation;
            // Applying colormap
            int i = std::round(value * (colormap.size() - 1));
            if (i < 0)
                i = 0;
            else if (i >= colormap.size())
                i = colormap.size() - 1;
            scanline[x] = colormap[i].rgb();
        }
    }
    setPixmap(QPixmap::fromImage(img));
}
