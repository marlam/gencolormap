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

#include "gui.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include <QImage>
#include <QtMath>

#include "colormapwidgets.hpp"
#include "colormap.hpp"


/* References */

static QString brewerlike_reference = QString("Relevant paper: "
        "M. Wijffelaars, R. Vliegen, J. J. van Wijk, E-J. van der Linden, "
        "<a href=\"http://dx.doi.org/10.1111/j.1467-8659.2008.01203.x\">Generating Color Palettes using Intuitive Parameters</a>, "
        "Computer Graphics Forum 27(3), May 2008.");

static QString isoluminant_reference = QString("");

static QString cubehelix_reference = QString("Relevant paper: "
        "D. A. Green, "
        "<a href=\"http://www.mrao.cam.ac.uk/~dag/CUBEHELIX/\">A colour scheme for the display of astronomical intensity</a>, "
        "Bulletin of the Astronomical Society of India 39(2), June 2011.");

static QString moreland_reference = QString("Relevant paper: "
        "K. Moreland, "
        "<a href=\"http://www.kennethmoreland.com/color-maps/\">Diverging Color Maps for Scientific Visualization</a>, "
        "Proc. Int. Symp. Visual Computing, December 2009."); // DOI 10.1007/978-3-642-10520-3_9.

static QString mcnames_references = QString("Relevant paper: "
        "J. McNames, "
        "<a href=\"http://dx.doi.org/10.1109/MSP.2006.1593340\">An Effective Color Scale for Simultaneous Color and Gray-Scale Publications</a>, "
        "IEEE Signal Processing Magazine 23(1), January 2006.");

/* ColorMapCombinedSliderSpinBox */

ColorMapCombinedSliderSpinBox::ColorMapCombinedSliderSpinBox(float minval, float maxval, float step) :
    _update_lock(false),
    minval(minval), maxval(maxval), step(step)
{
    slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(0);
    slider->setMaximum((maxval - minval) / step);
    slider->setSingleStep(step);

    spinbox = new QDoubleSpinBox();
    spinbox->setRange(minval, maxval);
    spinbox->setSingleStep(step);
    spinbox->setDecimals(std::log10(1.0f / step));

    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged()));
    connect(spinbox, SIGNAL(valueChanged(double)), this, SLOT(spinboxChanged()));
}

float ColorMapCombinedSliderSpinBox::value() const
{
    return spinbox->value();
}

void ColorMapCombinedSliderSpinBox::setValue(float v)
{
    _update_lock = true;
    spinbox->setValue(v);
    slider->setValue((v - minval) / step);
    _update_lock = false;
}

void ColorMapCombinedSliderSpinBox::sliderChanged()
{
    if (!_update_lock) {
        _update_lock = true;
        int i = slider->value();
        float v = i * step + minval;
        spinbox->setValue(v);
        _update_lock = false;
        emit valueChanged(value());
    }
}

void ColorMapCombinedSliderSpinBox::spinboxChanged()
{
    if (!_update_lock) {
        _update_lock = true;
        float v = spinbox->value();
        int i = (v - minval) / step;
        slider->setValue(i);
        _update_lock = false;
        emit valueChanged(value());
    }
}

/* ColorMapWidget */

ColorMapWidget::ColorMapWidget() : QWidget()
{
}

ColorMapWidget::~ColorMapWidget()
{
}

QImage ColorMapWidget::colorMapImage(int width, int height)
{
    QVector<QColor> colormap = colorMap();
    if (width <= 0)
        width = colormap.size();
    if (height <= 0)
        height = colormap.size();
    QImage img(width, height, QImage::Format_RGB32);
    bool y_direction = (height > width);
    if (y_direction) {
        for (int y = 0; y < height; y++) {
            float entry_height = height / static_cast<float>(colormap.size());
            int i = y / entry_height;
            QRgb rgb = colormap[i].rgb();
            QRgb* scanline = reinterpret_cast<QRgb*>(img.scanLine(height - 1 - y));
            for (int x = 0; x < width; x++)
                scanline[x] = rgb;
        }
    } else {
        for (int y = 0; y < height; y++) {
            QRgb* scanline = reinterpret_cast<QRgb*>(img.scanLine(y));
            for (int x = 0; x < width; x++) {
                float entry_width = width / static_cast<float>(colormap.size());
                int i = x / entry_width;
                scanline[x] = colormap[i].rgb();
            }
        }
    }
    return img;
}

/* Helper functions for ColorMap*Widget */

static QVector<QColor> toQColor(const QVector<unsigned char>& cm)
{
    QVector<QColor> colormap(cm.size() / 3);
    for (int i = 0; i < colormap.size(); i++)
        colormap[i] = QColor(cm[3 * i + 0], cm[3 * i + 1], cm[3 * i + 2]);
    return colormap;
}

static void hideWidgetButPreserveSize(QWidget* widget)
{
    QSizePolicy sp = widget->sizePolicy();
    sp.setRetainSizeWhenHidden(true);
    widget->setSizePolicy(sp);
    widget->hide();
}

/* ColorMapBrewerSequentialWidget */

ColorMapBrewerSequentialWidget::ColorMapBrewerSequentialWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* hue_label = new QLabel("Hue:");
    layout->addWidget(hue_label, 2, 0);
    _hue_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(_hue_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_hue_changer->spinbox, 2, 3);

    QLabel* divergence_label = new QLabel("Divergence:");
    layout->addWidget(divergence_label, 3, 0);
    ColorMapCombinedSliderSpinBox* divergence_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(divergence_changer->slider, 3, 1, 1, 2);
    layout->addWidget(divergence_changer->spinbox, 3, 3);
    hideWidgetButPreserveSize(divergence_label);
    hideWidgetButPreserveSize(divergence_changer->slider);
    hideWidgetButPreserveSize(divergence_changer->spinbox);

    QLabel* warmth_label = new QLabel("Warmth:");
    layout->addWidget(warmth_label, 4, 0);
    _warmth_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_warmth_changer->slider, 4, 1, 1, 2);
    layout->addWidget(_warmth_changer->spinbox, 4, 3);

    QLabel* contrast_label = new QLabel("Contrast:");
    layout->addWidget(contrast_label, 5, 0);
    _contrast_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_contrast_changer->slider, 5, 1, 1, 2);
    layout->addWidget(_contrast_changer->spinbox, 5, 3);

    QLabel* saturation_label = new QLabel("Saturation:");
    layout->addWidget(saturation_label, 6, 0);
    _saturation_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_saturation_changer->slider, 6, 1, 1, 2);
    layout->addWidget(_saturation_changer->spinbox, 6, 3);

    QLabel* brightness_label = new QLabel("Brightness:");
    layout->addWidget(brightness_label, 7, 0);
    _brightness_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_brightness_changer->slider, 7, 1, 1, 2);
    layout->addWidget(_brightness_changer->spinbox, 7, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 8, 0, 1, 4);
    layout->setRowStretch(8, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_warmth_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_contrast_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_brightness_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapBrewerSequentialWidget::~ColorMapBrewerSequentialWidget()
{
}

void ColorMapBrewerSequentialWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(256);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::BrewerSequentialDefaultHue));
    _warmth_changer->setValue(ColorMap::BrewerSequentialDefaultWarmth);
    _contrast_changer->setValue(ColorMap::BrewerSequentialDefaultContrast);
    _saturation_changer->setValue(ColorMap::BrewerSequentialDefaultSaturation);
    _brightness_changer->setValue(ColorMap::BrewerSequentialDefaultBrightness);
    _update_lock = false;
    update();
}

QVector<QColor> ColorMapBrewerSequentialWidget::colorMap() const
{
    int n;
    float h, c, s, b, w;
    parameters(n, h, c, s, b, w);
    QVector<unsigned char> colormap(3 * n);
    ColorMap::BrewerSequential(n, colormap.data(), h, c, s, b, w);
    return toQColor(colormap);
}

QString ColorMapBrewerSequentialWidget::reference() const
{
    return brewerlike_reference;
}

void ColorMapBrewerSequentialWidget::parameters(int& n, float& hue,
        float& contrast, float& saturation, float& brightness, float& warmth) const
{
    n = _n_spinbox->value();
    hue = qDegreesToRadians(_hue_changer->value());
    contrast = _contrast_changer->value();
    saturation = _saturation_changer->value();
    brightness = _brightness_changer->value();
    warmth = _warmth_changer->value();
}

void ColorMapBrewerSequentialWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapBrewerDivergingWidget */

ColorMapBrewerDivergingWidget::ColorMapBrewerDivergingWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* hue_label = new QLabel("Hue:");
    layout->addWidget(hue_label, 2, 0);
    _hue_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(_hue_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_hue_changer->spinbox, 2, 3);

    QLabel* divergence_label = new QLabel("Divergence:");
    layout->addWidget(divergence_label, 3, 0);
    _divergence_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(_divergence_changer->slider, 3, 1, 1, 2);
    layout->addWidget(_divergence_changer->spinbox, 3, 3);

    QLabel* warmth_label = new QLabel("Warmth:");
    layout->addWidget(warmth_label, 4, 0);
    _warmth_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_warmth_changer->slider, 4, 1, 1, 2);
    layout->addWidget(_warmth_changer->spinbox, 4, 3);

    QLabel* contrast_label = new QLabel("Contrast:");
    layout->addWidget(contrast_label, 5, 0);
    _contrast_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_contrast_changer->slider, 5, 1, 1, 2);
    layout->addWidget(_contrast_changer->spinbox, 5, 3);

    QLabel* saturation_label = new QLabel("Saturation:");
    layout->addWidget(saturation_label, 6, 0);
    _saturation_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_saturation_changer->slider, 6, 1, 1, 2);
    layout->addWidget(_saturation_changer->spinbox, 6, 3);

    QLabel* brightness_label = new QLabel("Brightness:");
    layout->addWidget(brightness_label, 7, 0);
    _brightness_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_brightness_changer->slider, 7, 1, 1, 2);
    layout->addWidget(_brightness_changer->spinbox, 7, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 8, 0, 1, 4);
    layout->setRowStretch(8, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_divergence_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_warmth_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_contrast_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_brightness_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapBrewerDivergingWidget::~ColorMapBrewerDivergingWidget()
{
}

void ColorMapBrewerDivergingWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(257);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::BrewerDivergingDefaultHue));
    _divergence_changer->setValue(qRadiansToDegrees(ColorMap::BrewerDivergingDefaultDivergence));
    _warmth_changer->setValue(ColorMap::BrewerDivergingDefaultWarmth);
    _contrast_changer->setValue(ColorMap::BrewerDivergingDefaultContrast);
    _saturation_changer->setValue(ColorMap::BrewerDivergingDefaultSaturation);
    _brightness_changer->setValue(ColorMap::BrewerDivergingDefaultBrightness);
    _update_lock = false;
    update();
}

QVector<QColor> ColorMapBrewerDivergingWidget::colorMap() const
{
    int n;
    float h, d, c, s, b, w;
    parameters(n, h, d, c, s, b, w);
    QVector<unsigned char> colormap(3 * n);
    ColorMap::BrewerDiverging(n, colormap.data(), h, d, c, s, b, w);
    return toQColor(colormap);
}

QString ColorMapBrewerDivergingWidget::reference() const
{
    return brewerlike_reference;
}

void ColorMapBrewerDivergingWidget::parameters(int& n, float& hue, float& divergence,
        float& contrast, float& saturation, float& brightness, float& warmth) const
{
    n = _n_spinbox->value();
    hue = qDegreesToRadians(_hue_changer->value());
    divergence = qDegreesToRadians(_divergence_changer->value());
    contrast = _contrast_changer->value();
    saturation = _saturation_changer->value();
    brightness = _brightness_changer->value();
    warmth = _warmth_changer->value();
}

void ColorMapBrewerDivergingWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapBrewerQualitativeWidget */

ColorMapBrewerQualitativeWidget::ColorMapBrewerQualitativeWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* hue_label = new QLabel("Hue:");
    layout->addWidget(hue_label, 2, 0);
    _hue_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(_hue_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_hue_changer->spinbox, 2, 3);

    QLabel* divergence_label = new QLabel("Divergence:");
    layout->addWidget(divergence_label, 3, 0);
    _divergence_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(_divergence_changer->slider, 3, 1, 1, 2);
    layout->addWidget(_divergence_changer->spinbox, 3, 3);

    QLabel* warmth_label = new QLabel("Warmth:");
    layout->addWidget(warmth_label, 4, 0);
    ColorMapCombinedSliderSpinBox* warmth_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(warmth_changer->slider, 4, 1, 1, 2);
    layout->addWidget(warmth_changer->spinbox, 4, 3);
    hideWidgetButPreserveSize(warmth_label);
    hideWidgetButPreserveSize(warmth_changer->slider);
    hideWidgetButPreserveSize(warmth_changer->spinbox);

    QLabel* contrast_label = new QLabel("Contrast:");
    layout->addWidget(contrast_label, 5, 0);
    _contrast_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_contrast_changer->slider, 5, 1, 1, 2);
    layout->addWidget(_contrast_changer->spinbox, 5, 3);

    QLabel* saturation_label = new QLabel("Saturation:");
    layout->addWidget(saturation_label, 6, 0);
    _saturation_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_saturation_changer->slider, 6, 1, 1, 2);
    layout->addWidget(_saturation_changer->spinbox, 6, 3);

    QLabel* brightness_label = new QLabel("Brightness:");
    layout->addWidget(brightness_label, 7, 0);
    _brightness_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_brightness_changer->slider, 7, 1, 1, 2);
    layout->addWidget(_brightness_changer->spinbox, 7, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 8, 0, 1, 4);
    layout->setRowStretch(8, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_divergence_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_contrast_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_brightness_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapBrewerQualitativeWidget::~ColorMapBrewerQualitativeWidget()
{
}

void ColorMapBrewerQualitativeWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(9);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::BrewerQualitativeDefaultHue));
    _divergence_changer->setValue(qRadiansToDegrees(ColorMap::BrewerQualitativeDefaultDivergence));
    _contrast_changer->setValue(ColorMap::BrewerQualitativeDefaultContrast);
    _saturation_changer->setValue(ColorMap::BrewerQualitativeDefaultSaturation);
    _brightness_changer->setValue(ColorMap::BrewerQualitativeDefaultBrightness);
    _update_lock = false;
    update();
}

QVector<QColor> ColorMapBrewerQualitativeWidget::colorMap() const
{
    int n;
    float h, d, c, s, b;
    parameters(n, h, d, c, s, b);
    QVector<unsigned char> colormap(3 * n);
    ColorMap::BrewerQualitative(n, colormap.data(), h, d, c, s, b);
    return toQColor(colormap);
}

QString ColorMapBrewerQualitativeWidget::reference() const
{
    return brewerlike_reference;
}

void ColorMapBrewerQualitativeWidget::parameters(int& n, float& hue, float& divergence,
        float& contrast, float& saturation, float& brightness) const
{
    n = _n_spinbox->value();
    hue = qDegreesToRadians(_hue_changer->value());
    divergence = qDegreesToRadians(_divergence_changer->value());
    contrast = _contrast_changer->value();
    saturation = _saturation_changer->value();
    brightness = _brightness_changer->value();
}

void ColorMapBrewerQualitativeWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapIsoluminantSequentialWidget */

ColorMapIsoluminantSequentialWidget::ColorMapIsoluminantSequentialWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* luminance_label = new QLabel("Luminance:");
    layout->addWidget(luminance_label, 2, 0);
    _luminance_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_luminance_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_luminance_changer->spinbox, 2, 3);

    QLabel* saturation_label = new QLabel("Saturation:");
    layout->addWidget(saturation_label, 3, 0);
    _saturation_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_saturation_changer->slider, 3, 1, 1, 2);
    layout->addWidget(_saturation_changer->spinbox, 3, 3);

    QLabel* hue_label = new QLabel("Hue:");
    layout->addWidget(hue_label, 4, 0);
    _hue_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(_hue_changer->slider, 4, 1, 1, 2);
    layout->addWidget(_hue_changer->spinbox, 4, 3);

    QLabel* divergence_label = new QLabel("Divergence:");
    layout->addWidget(divergence_label, 5, 0);
    ColorMapCombinedSliderSpinBox* divergence_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(divergence_changer->slider, 5, 1, 1, 2);
    layout->addWidget(divergence_changer->spinbox, 5, 3);
    hideWidgetButPreserveSize(divergence_label);
    hideWidgetButPreserveSize(divergence_changer->slider);
    hideWidgetButPreserveSize(divergence_changer->spinbox);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 6, 0, 1, 4);
    layout->setRowStretch(6, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_luminance_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapIsoluminantSequentialWidget::~ColorMapIsoluminantSequentialWidget()
{
}

void ColorMapIsoluminantSequentialWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(256);
    _luminance_changer->setValue(ColorMap::IsoluminantSequentialDefaultLuminance);
    _saturation_changer->setValue(ColorMap::IsoluminantSequentialDefaultSaturation);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::IsoluminantSequentialDefaultHue));
    _update_lock = false;
    update();
}

QVector<QColor> ColorMapIsoluminantSequentialWidget::colorMap() const
{
    int n;
    float l, s, h;
    parameters(n, l, s, h);
    QVector<unsigned char> colormap(3 * n);
    ColorMap::IsoluminantSequential(n, colormap.data(), l, s, h);
    return toQColor(colormap);
}

QString ColorMapIsoluminantSequentialWidget::reference() const
{
    return isoluminant_reference;
}

void ColorMapIsoluminantSequentialWidget::parameters(int& n, float& luminance, float& saturation, float& hue) const
{
    n = _n_spinbox->value();
    luminance = _luminance_changer->value();
    saturation = _saturation_changer->value();
    hue = qDegreesToRadians(_hue_changer->value());
}

void ColorMapIsoluminantSequentialWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapIsoluminantDivergingWidget */

ColorMapIsoluminantDivergingWidget::ColorMapIsoluminantDivergingWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* luminance_label = new QLabel("Luminance:");
    layout->addWidget(luminance_label, 2, 0);
    _luminance_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_luminance_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_luminance_changer->spinbox, 2, 3);

    QLabel* saturation_label = new QLabel("Saturation:");
    layout->addWidget(saturation_label, 3, 0);
    _saturation_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_saturation_changer->slider, 3, 1, 1, 2);
    layout->addWidget(_saturation_changer->spinbox, 3, 3);

    QLabel* hue_label = new QLabel("Hue:");
    layout->addWidget(hue_label, 4, 0);
    _hue_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(_hue_changer->slider, 4, 1, 1, 2);
    layout->addWidget(_hue_changer->spinbox, 4, 3);

    QLabel* divergence_label = new QLabel("Divergence:");
    layout->addWidget(divergence_label, 5, 0);
    _divergence_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(_divergence_changer->slider, 5, 1, 1, 2);
    layout->addWidget(_divergence_changer->spinbox, 5, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 6, 0, 1, 4);
    layout->setRowStretch(6, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_luminance_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_divergence_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapIsoluminantDivergingWidget::~ColorMapIsoluminantDivergingWidget()
{
}

void ColorMapIsoluminantDivergingWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(257);
    _luminance_changer->setValue(ColorMap::IsoluminantDivergingDefaultLuminance);
    _saturation_changer->setValue(ColorMap::IsoluminantDivergingDefaultSaturation);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::IsoluminantDivergingDefaultHue));
    _divergence_changer->setValue(qRadiansToDegrees(ColorMap::IsoluminantDivergingDefaultDivergence));
    _update_lock = false;
    update();
}

QVector<QColor> ColorMapIsoluminantDivergingWidget::colorMap() const
{
    int n;
    float l, s, h, d;
    parameters(n, l, s, h, d);
    QVector<unsigned char> colormap(3 * n);
    ColorMap::IsoluminantDiverging(n, colormap.data(), l, s, h, d);
    return toQColor(colormap);
}

QString ColorMapIsoluminantDivergingWidget::reference() const
{
    return isoluminant_reference;
}

void ColorMapIsoluminantDivergingWidget::parameters(int& n, float& luminance, float& saturation, float& hue, float& divergence) const
{
    n = _n_spinbox->value();
    luminance = _luminance_changer->value();
    saturation = _saturation_changer->value();
    hue = qDegreesToRadians(_hue_changer->value());
    divergence = qDegreesToRadians(_divergence_changer->value());
}

void ColorMapIsoluminantDivergingWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapIsoluminantQualitativeWidget */

ColorMapIsoluminantQualitativeWidget::ColorMapIsoluminantQualitativeWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* luminance_label = new QLabel("Luminance:");
    layout->addWidget(luminance_label, 2, 0);
    _luminance_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_luminance_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_luminance_changer->spinbox, 2, 3);

    QLabel* saturation_label = new QLabel("Saturation:");
    layout->addWidget(saturation_label, 3, 0);
    _saturation_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_saturation_changer->slider, 3, 1, 1, 2);
    layout->addWidget(_saturation_changer->spinbox, 3, 3);

    QLabel* hue_label = new QLabel("Hue:");
    layout->addWidget(hue_label, 4, 0);
    _hue_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(_hue_changer->slider, 4, 1, 1, 2);
    layout->addWidget(_hue_changer->spinbox, 4, 3);

    QLabel* divergence_label = new QLabel("Divergence:");
    layout->addWidget(divergence_label, 5, 0);
    _divergence_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(_divergence_changer->slider, 5, 1, 1, 2);
    layout->addWidget(_divergence_changer->spinbox, 5, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 6, 0, 1, 4);
    layout->setRowStretch(6, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_luminance_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_divergence_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapIsoluminantQualitativeWidget::~ColorMapIsoluminantQualitativeWidget()
{
}

void ColorMapIsoluminantQualitativeWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(9);
    _luminance_changer->setValue(ColorMap::IsoluminantQualitativeDefaultLuminance);
    _saturation_changer->setValue(ColorMap::IsoluminantQualitativeDefaultSaturation);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::IsoluminantQualitativeDefaultHue));
    _divergence_changer->setValue(qRadiansToDegrees(ColorMap::IsoluminantQualitativeDefaultDivergence));
    _update_lock = false;
    update();
}

QVector<QColor> ColorMapIsoluminantQualitativeWidget::colorMap() const
{
    int n;
    float l, s, h, d;
    parameters(n, l, s, h, d);
    QVector<unsigned char> colormap(3 * n);
    ColorMap::IsoluminantQualitative(n, colormap.data(), l, s, h, d);
    return toQColor(colormap);
}

QString ColorMapIsoluminantQualitativeWidget::reference() const
{
    return isoluminant_reference;
}

void ColorMapIsoluminantQualitativeWidget::parameters(int& n, float& luminance, float& saturation, float& hue, float& divergence) const
{
    n = _n_spinbox->value();
    luminance = _luminance_changer->value();
    saturation = _saturation_changer->value();
    hue = qDegreesToRadians(_hue_changer->value());
    divergence = qDegreesToRadians(_divergence_changer->value());
}

void ColorMapIsoluminantQualitativeWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapCubeHelixWidget */

ColorMapCubeHelixWidget::ColorMapCubeHelixWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* hue_label = new QLabel("Hue:");
    layout->addWidget(hue_label, 2, 0);
    _hue_changer = new ColorMapCombinedSliderSpinBox(0, 180, 1);
    layout->addWidget(_hue_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_hue_changer->spinbox, 2, 3);

    QLabel* rotations_label = new QLabel("Rotations:");
    layout->addWidget(rotations_label, 3, 0);
    _rotations_changer = new ColorMapCombinedSliderSpinBox(-5.0f, +5.0f, 0.1f);
    layout->addWidget(_rotations_changer->slider, 3, 1, 1, 2);
    layout->addWidget(_rotations_changer->spinbox, 3, 3);

    QLabel* saturation_label = new QLabel("Saturation:");
    layout->addWidget(saturation_label, 4, 0);
    _saturation_changer = new ColorMapCombinedSliderSpinBox(0.0f, 2.0f, 0.1f);
    layout->addWidget(_saturation_changer->slider, 4, 1, 1, 2);
    layout->addWidget(_saturation_changer->spinbox, 4, 3);

    QLabel* gamma_label = new QLabel("Gamma:");
    layout->addWidget(gamma_label, 5, 0);
    _gamma_changer = new ColorMapCombinedSliderSpinBox(0.3f, 3.0f, 0.1f);
    layout->addWidget(_gamma_changer->slider, 5, 1, 1, 2);
    layout->addWidget(_gamma_changer->spinbox, 5, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 6, 0, 1, 4);
    layout->setRowStretch(6, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_rotations_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_gamma_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapCubeHelixWidget::~ColorMapCubeHelixWidget()
{
}

void ColorMapCubeHelixWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(256);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::CubeHelixDefaultHue));
    _rotations_changer->setValue(ColorMap::CubeHelixDefaultRotations);
    _saturation_changer->setValue(ColorMap::CubeHelixDefaultSaturation);
    _gamma_changer->setValue(ColorMap::CubeHelixDefaultGamma);
    _update_lock = false;
    update();
}

QVector<QColor> ColorMapCubeHelixWidget::colorMap() const
{
    int n;
    float h, r, s, g;
    parameters(n, h, r, s, g);
    QVector<unsigned char> colormap(3 * n);
    ColorMap::CubeHelix(n, colormap.data(), h, r, s, g);
    return toQColor(colormap);
}

QString ColorMapCubeHelixWidget::reference() const
{
    return cubehelix_reference;
}

void ColorMapCubeHelixWidget::parameters(int& n, float& hue,
        float& rotations, float& saturation, float& gamma) const
{
    n = _n_spinbox->value();
    hue = qDegreesToRadians(_hue_changer->value());
    rotations = _rotations_changer->value();
    saturation = _saturation_changer->value();
    gamma = _gamma_changer->value();
}

void ColorMapCubeHelixWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapMorelandWidget */

ColorMapMorelandWidget::ColorMapMorelandWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* color0_label = new QLabel("First color:");
    layout->addWidget(color0_label, 2, 0);
    _color0_button = new QPushButton;
    layout->addWidget(_color0_button, 2, 1, 1, 3);

    QLabel* color1_label = new QLabel("Last color:");
    layout->addWidget(color1_label, 3, 0);
    _color1_button = new QPushButton;
    layout->addWidget(_color1_button, 3, 1, 1, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 4, 0, 1, 4);
    layout->setRowStretch(4, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_color0_button, SIGNAL(clicked(bool)), this, SLOT(chooseColor0()));
    connect(_color1_button, SIGNAL(clicked(bool)), this, SLOT(chooseColor1()));
    reset();
}

ColorMapMorelandWidget::~ColorMapMorelandWidget()
{
}

static void setButtonColor(QPushButton* button, const QColor& color)
{
    QPixmap pixmap(button->iconSize());
    pixmap.fill(color);
    button->setIcon(QIcon(pixmap));
}

static QColor getButtonColor(QPushButton* button)
{
    return button->icon().pixmap(1, 1).toImage().pixel(0, 0);
}

void ColorMapMorelandWidget::chooseColor0()
{
    QColor c = QColorDialog::getColor(getButtonColor(_color0_button), this);
    if (c.isValid()) {
        setButtonColor(_color0_button, c);
        update();
    }
}

void ColorMapMorelandWidget::chooseColor1()
{
    QColor c = QColorDialog::getColor(getButtonColor(_color1_button), this);
    if (c.isValid()) {
        setButtonColor(_color1_button, c);
        update();
    }
}

void ColorMapMorelandWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(257);
    setButtonColor(_color0_button, QColor(
                ColorMap::MorelandDefaultR0,
                ColorMap::MorelandDefaultG0,
                ColorMap::MorelandDefaultB0));
    setButtonColor(_color1_button, QColor(
                ColorMap::MorelandDefaultR1,
                ColorMap::MorelandDefaultG1,
                ColorMap::MorelandDefaultB1));
    _update_lock = false;
    update();
}

QVector<QColor> ColorMapMorelandWidget::colorMap() const
{
    int n;
    unsigned char r0, g0, b0, r1, g1, b1;
    parameters(n, r0, g0, b0, r1, g1, b1);
    QVector<unsigned char> colormap(3 * n);
    ColorMap::Moreland(n, colormap.data(), r0, g0, b0, r1, g1, b1);
    return toQColor(colormap);
}

QString ColorMapMorelandWidget::reference() const
{
    return moreland_reference;
}

void ColorMapMorelandWidget::parameters(int& n,
        unsigned char& r0, unsigned char& g0, unsigned char& b0,
        unsigned char& r1, unsigned char& g1, unsigned char& b1) const
{
    n = _n_spinbox->value();
    QColor c0 = getButtonColor(_color0_button);
    r0 = c0.red();
    g0 = c0.green();
    b0 = c0.blue();
    QColor c1 = getButtonColor(_color1_button);
    r1 = c1.red();
    g1 = c1.green();
    b1 = c1.blue();
}

void ColorMapMorelandWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapMcNamesWidget */

ColorMapMcNamesWidget::ColorMapMcNamesWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* periods_label = new QLabel("Periods:");
    layout->addWidget(periods_label, 2, 0);
    _periods_changer = new ColorMapCombinedSliderSpinBox(0.1f, 5.0f, 0.1f);
    layout->addWidget(_periods_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_periods_changer->spinbox, 2, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 3, 0, 1, 4);
    layout->setRowStretch(3, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_periods_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapMcNamesWidget::~ColorMapMcNamesWidget()
{
}

void ColorMapMcNamesWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(256);
    _periods_changer->setValue(ColorMap::McNamesDefaultPeriods);
    _update_lock = false;
    update();
}

QVector<QColor> ColorMapMcNamesWidget::colorMap() const
{
    int n;
    float p;
    parameters(n, p);
    QVector<unsigned char> colormap(3 * n);
    ColorMap::McNames(n, colormap.data(), p);
    return toQColor(colormap);
}

QString ColorMapMcNamesWidget::reference() const
{
    return mcnames_references;
}

void ColorMapMcNamesWidget::parameters(int& n, float& p) const
{
    n = _n_spinbox->value();
    p = _periods_changer->value();
}

void ColorMapMcNamesWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}
