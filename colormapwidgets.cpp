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

#include "gui.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include <QImage>
#include <QListWidget>
#include <QPushButton>
#include <QColorDialog>
#include <QtMath>

#include "colormapwidgets.hpp"
#include "colormap.hpp"


/* References */

static QString brewerlike_reference = QString("Relevant paper: "
        "M. Wijffelaars, R. Vliegen, J. J. van Wijk, E-J. van der Linden, "
        "<a href=\"http://dx.doi.org/10.1111/j.1467-8659.2008.01203.x\">Generating Color Palettes using Intuitive Parameters</a>, "
        "Computer Graphics Forum 27(3), May 2008.");

static QString plseq_lightness_reference = QString(
        "Perceptually linear, constant hue, varying lightness and saturation.<br>"
        "Computed in CIELUV/LCH color space.");

static QString plseq_saturation_reference = QString(
        "Perceptually linear, constant lightness and hue, varying saturation.<br>"
        "Computed in CIELUV/LCH color space.");

static QString plseq_rainbow_reference = QString(
        "Perceptually linear, varying lightness, saturation, and hue (rainbow).<br>"
        "Computed in CIELUV/LCH color space.");

static QString plseq_blackbody_reference = QString(
        "Perceptually linear, varying lightness, saturation, and hue (black body at increasing temperatures).<br>"
        "Computed in CIELUV/LCH color space.");

static QString plseq_multihue_reference = QString(
        "Perceptually linear, varying lightness, saturation, and hue (user definable).<br>"
        "Computed in CIELUV/LCH color space.");

static QString pldiv_lightness_reference = QString(
        "Perceptually linear, constant hues, varying lightness and saturation.<br>"
        "Computed in CIELUV/LCH color space.");

static QString pldiv_saturation_reference = QString(
        "Perceptually linear, constant lightness and hues, varying saturation.<br>"
        "Computed in CIELUV/LCH color space.");

static QString plqual_hue_reference = QString(
        "Perceptually linear, constant lightness and saturation, evenly distributed hue.<br>"
        "Computed in CIELUV/LCH color space.");

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

QImage ColorMapWidget::colorMapImage(const QVector<unsigned char>& colormap, int width, int height)
{
    QVector<QColor> qcolormap(colormap.size() / 3);
    for (int i = 0; i < qcolormap.size(); i++)
        qcolormap[i] = QColor(colormap[3 * i + 0], colormap[3 * i + 1], colormap[3 * i + 2]);

    if (width <= 0)
        width = qcolormap.size();
    if (height <= 0)
        height = qcolormap.size();
    QImage img(width, height, QImage::Format_RGB32);
    bool y_direction = (height > width);
    if (y_direction) {
        for (int y = 0; y < height; y++) {
            float entry_height = height / static_cast<float>(qcolormap.size());
            int i = y / entry_height;
            QRgb rgb = qcolormap[i].rgb();
            QRgb* scanline = reinterpret_cast<QRgb*>(img.scanLine(height - 1 - y));
            for (int x = 0; x < width; x++)
                scanline[x] = rgb;
        }
    } else {
        for (int y = 0; y < height; y++) {
            QRgb* scanline = reinterpret_cast<QRgb*>(img.scanLine(y));
            for (int x = 0; x < width; x++) {
                float entry_width = width / static_cast<float>(qcolormap.size());
                int i = x / entry_width;
                scanline[x] = qcolormap[i].rgb();
            }
        }
    }
    return img;
}

/* Helper functions for ColorMap*Widget */

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

QVector<unsigned char> ColorMapBrewerSequentialWidget::colorMap(int* clipped) const
{
    int n;
    float h, c, s, b, w;
    parameters(n, h, c, s, b, w);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::BrewerSequential(n, colormap.data(), h, c, s, b, w);
    if (clipped)
        *clipped = cl;
    return colormap;
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

QVector<unsigned char> ColorMapBrewerDivergingWidget::colorMap(int* clipped) const
{
    int n;
    float h, d, c, s, b, w;
    parameters(n, h, d, c, s, b, w);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::BrewerDiverging(n, colormap.data(), h, d, c, s, b, w);
    if (clipped)
        *clipped = cl;
    return colormap;
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
    _n_spinbox->setValue(256);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::BrewerQualitativeDefaultHue));
    _divergence_changer->setValue(qRadiansToDegrees(ColorMap::BrewerQualitativeDefaultDivergence));
    _contrast_changer->setValue(ColorMap::BrewerQualitativeDefaultContrast);
    _saturation_changer->setValue(ColorMap::BrewerQualitativeDefaultSaturation);
    _brightness_changer->setValue(ColorMap::BrewerQualitativeDefaultBrightness);
    _update_lock = false;
    update();
}

QVector<unsigned char> ColorMapBrewerQualitativeWidget::colorMap(int* clipped) const
{
    int n;
    float h, d, c, s, b;
    parameters(n, h, d, c, s, b);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::BrewerQualitative(n, colormap.data(), h, d, c, s, b);
    if (clipped)
        *clipped = cl;
    return colormap;
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

/* ColorMapPLSequentialLightnessWidget */

ColorMapPLSequentialLightnessWidget::ColorMapPLSequentialLightnessWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* saturation_label = new QLabel("Saturation:");
    layout->addWidget(saturation_label, 2, 0);
    _saturation_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_saturation_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_saturation_changer->spinbox, 2, 3);

    QLabel* hue_label = new QLabel("Hue:");
    layout->addWidget(hue_label, 3, 0);
    _hue_changer = new ColorMapCombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(_hue_changer->slider, 3, 1, 1, 2);
    layout->addWidget(_hue_changer->spinbox, 3, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 4, 0, 1, 4);
    layout->setRowStretch(4, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapPLSequentialLightnessWidget::~ColorMapPLSequentialLightnessWidget()
{
}

void ColorMapPLSequentialLightnessWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(256);
    _saturation_changer->setValue(ColorMap::PLSequentialLightnessDefaultSaturation);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::PLSequentialLightnessDefaultHue));
    _update_lock = false;
    update();
}

QVector<unsigned char> ColorMapPLSequentialLightnessWidget::colorMap(int* clipped) const
{
    int n;
    float s, h;
    parameters(n, s, h);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::PLSequentialLightness(n, colormap.data(), s, h);
    if (clipped)
        *clipped = cl;
    return colormap;
}

QString ColorMapPLSequentialLightnessWidget::reference() const
{
    return plseq_lightness_reference;
}

void ColorMapPLSequentialLightnessWidget::parameters(int& n, float& saturation, float& hue) const
{
    n = _n_spinbox->value();
    saturation = _saturation_changer->value();
    hue = qDegreesToRadians(_hue_changer->value());
}

void ColorMapPLSequentialLightnessWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapPLSequentialSaturationWidget */

ColorMapPLSequentialSaturationWidget::ColorMapPLSequentialSaturationWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* lightness_label = new QLabel("Lightness:");
    layout->addWidget(lightness_label, 2, 0);
    _lightness_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_lightness_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_lightness_changer->spinbox, 2, 3);

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

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 5, 0, 1, 4);
    layout->setRowStretch(5, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_lightness_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapPLSequentialSaturationWidget::~ColorMapPLSequentialSaturationWidget()
{
}

void ColorMapPLSequentialSaturationWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(256);
    _lightness_changer->setValue(ColorMap::PLSequentialSaturationDefaultLightness);
    _saturation_changer->setValue(ColorMap::PLSequentialSaturationDefaultSaturation);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::PLSequentialSaturationDefaultHue));
    _update_lock = false;
    update();
}

QVector<unsigned char> ColorMapPLSequentialSaturationWidget::colorMap(int* clipped) const
{
    int n;
    float l, s, h;
    parameters(n, l, s, h);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::PLSequentialSaturation(n, colormap.data(), l, s, h);
    if (clipped)
        *clipped = cl;
    return colormap;
}

QString ColorMapPLSequentialSaturationWidget::reference() const
{
    return plseq_saturation_reference;
}

void ColorMapPLSequentialSaturationWidget::parameters(int& n, float& lightness, float& saturation, float& hue) const
{
    n = _n_spinbox->value();
    lightness = _lightness_changer->value();
    saturation = _saturation_changer->value();
    hue = qDegreesToRadians(_hue_changer->value());
}

void ColorMapPLSequentialSaturationWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapPLSequentialRainbowWidget */

ColorMapPLSequentialRainbowWidget::ColorMapPLSequentialRainbowWidget() :
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

    QLabel* rotations_label = new QLabel("Rotations:");
    layout->addWidget(rotations_label, 3, 0);
    _rotations_changer = new ColorMapCombinedSliderSpinBox(-5.0f, +5.0f, 0.1f);
    layout->addWidget(_rotations_changer->slider, 3, 1, 1, 2);
    layout->addWidget(_rotations_changer->spinbox, 3, 3);

    QLabel* saturation_label = new QLabel("Saturation:");
    layout->addWidget(saturation_label, 4, 0);
    _saturation_changer = new ColorMapCombinedSliderSpinBox(0.0f, 5.0f, 0.1f);
    layout->addWidget(_saturation_changer->slider, 4, 1, 1, 2);
    layout->addWidget(_saturation_changer->spinbox, 4, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 5, 0, 1, 4);
    layout->setRowStretch(5, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_rotations_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapPLSequentialRainbowWidget::~ColorMapPLSequentialRainbowWidget()
{
}

void ColorMapPLSequentialRainbowWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(256);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::PLSequentialRainbowDefaultHue));
    _rotations_changer->setValue(ColorMap::PLSequentialRainbowDefaultRotations);
    _saturation_changer->setValue(ColorMap::PLSequentialRainbowDefaultSaturation);
    _update_lock = false;
    update();
}

QVector<unsigned char> ColorMapPLSequentialRainbowWidget::colorMap(int* clipped) const
{
    int n;
    float h, r, s;
    parameters(n, h, r, s);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::PLSequentialRainbow(n, colormap.data(), h, r, s);
    if (clipped)
        *clipped = cl;
    return colormap;
}

QString ColorMapPLSequentialRainbowWidget::reference() const
{
    return plseq_rainbow_reference;
}

void ColorMapPLSequentialRainbowWidget::parameters(int& n, float& hue,
        float& rotations, float& saturation) const
{
    n = _n_spinbox->value();
    hue = qDegreesToRadians(_hue_changer->value());
    rotations = _rotations_changer->value();
    saturation = _saturation_changer->value();
}

void ColorMapPLSequentialRainbowWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapPLSequentialBlackBodyWidget */

ColorMapPLSequentialBlackBodyWidget::ColorMapPLSequentialBlackBodyWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 10240);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* temperature_label = new QLabel("Temperature (K):");
    layout->addWidget(temperature_label, 2, 0);
    _temperature_changer = new ColorMapCombinedSliderSpinBox(250.0f, 20000.0f, 100.0f);
    layout->addWidget(_temperature_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_temperature_changer->spinbox, 2, 3);

    QLabel* range_label = new QLabel("Range (K):");
    layout->addWidget(range_label, 3, 0);
    _range_changer = new ColorMapCombinedSliderSpinBox(0.0f, 20000.0f, 100.0f);
    layout->addWidget(_range_changer->slider, 3, 1, 1, 2);
    layout->addWidget(_range_changer->spinbox, 3, 3);

    QLabel* saturation_label = new QLabel("Saturation:");
    layout->addWidget(saturation_label, 4, 0);
    _saturation_changer = new ColorMapCombinedSliderSpinBox(0.0f, 5.0f, 0.1f);
    layout->addWidget(_saturation_changer->slider, 4, 1, 1, 2);
    layout->addWidget(_saturation_changer->spinbox, 4, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 5, 0, 1, 4);
    layout->setRowStretch(5, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_temperature_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_range_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapPLSequentialBlackBodyWidget::~ColorMapPLSequentialBlackBodyWidget()
{
}

void ColorMapPLSequentialBlackBodyWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(256);
    _temperature_changer->setValue(ColorMap::PLSequentialBlackBodyDefaultTemperature);
    _range_changer->setValue(ColorMap::PLSequentialBlackBodyDefaultRange);
    _saturation_changer->setValue(ColorMap::PLSequentialBlackBodyDefaultSaturation);
    _update_lock = false;
    update();
}

QVector<unsigned char> ColorMapPLSequentialBlackBodyWidget::colorMap(int* clipped) const
{
    int n;
    float t, r, s;
    parameters(n, t, r, s);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::PLSequentialBlackBody(n, colormap.data(), t, r, s);
    if (clipped)
        *clipped = cl;
    return colormap;
}

QString ColorMapPLSequentialBlackBodyWidget::reference() const
{
    return plseq_blackbody_reference;
}

void ColorMapPLSequentialBlackBodyWidget::parameters(int& n, float& temperature, float& range, float& saturation) const
{
    n = _n_spinbox->value();
    temperature = _temperature_changer->value();
    range = _range_changer->value();
    saturation = _saturation_changer->value();
}

void ColorMapPLSequentialBlackBodyWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapPLSequentialMultiHueWidget */

static float colorToHue(QColor color)
{
    return color.hsvHueF() * 2.0f * static_cast<float>(M_PI);
}

static QColor hueToColor(float hue)
{
    return QColor::fromHsvF(hue / (2.0f * static_cast<float>(M_PI)), 1.0f, 1.0f);
}

ColorMapPLSequentialMultiHueWidget::ColorMapPLSequentialMultiHueWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 10240);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* hue_list_label = new QLabel("Hues:");
    layout->addWidget(hue_list_label, 2, 0);
    _hue_list_widget = new QListWidget;
    _hue_list_widget->setFlow(QListView::LeftToRight);
    _hue_list_widget->setMaximumHeight(2 * _n_spinbox->sizeHint().height());
    layout->addWidget(_hue_list_widget, 2, 1, 1, 3);

    QGridLayout *hue_layout = new QGridLayout;
    _hue_button_color = hueToColor(0.0f);
    _hue_button = new QPushButton;
    _hue_button->setFixedSize(QSize(_hue_button->sizeHint().width() / 2, _hue_button->sizeHint().height()));
    updateHueButton();
    connect(_hue_button, SIGNAL(clicked()), this, SLOT(hueButtonClicked()));
    hue_layout->addWidget(_hue_button, 0, 0);
    _pos_spinbox = new QDoubleSpinBox;
    _pos_spinbox->setRange(0.0f, 1.0f);
    _pos_spinbox->setSingleStep(0.01f);
    _pos_spinbox->setDecimals(2);
    hue_layout->addWidget(_pos_spinbox, 0, 1);
    QPushButton* add_hue_btn = new QPushButton("Add");
    connect(add_hue_btn, SIGNAL(clicked()), this, SLOT(addHue()));
    hue_layout->addWidget(add_hue_btn, 0, 2);
    hue_layout->addItem(new QSpacerItem(0, 0), 0, 3);
    QPushButton* remove_hue_btn = new QPushButton("Remove");
    connect(remove_hue_btn, SIGNAL(clicked()), this, SLOT(removeHue()));
    hue_layout->addWidget(remove_hue_btn, 0, 4);
    hue_layout->setColumnStretch(3, 1);
    layout->addLayout(hue_layout, 3, 1, 1, 3);

    QLabel* l0_label = new QLabel("Lightness at start:");
    layout->addWidget(l0_label, 4, 0);
    _l0_changer = new ColorMapCombinedSliderSpinBox(0.0f, 1.0f, 0.01f);
    layout->addWidget(_l0_changer->slider, 4, 1, 1, 2);
    layout->addWidget(_l0_changer->spinbox, 4, 3);

    QLabel* s0_label = new QLabel("Saturation at start:");
    layout->addWidget(s0_label, 5, 0);
    _s0_changer = new ColorMapCombinedSliderSpinBox(0.0f, 1.0f, 0.01f);
    layout->addWidget(_s0_changer->slider, 5, 1, 1, 2);
    layout->addWidget(_s0_changer->spinbox, 5, 3);

    QLabel* l1_label = new QLabel("Lightness at end:");
    layout->addWidget(l1_label, 6, 0);
    _l1_changer = new ColorMapCombinedSliderSpinBox(0.0f, 1.0f, 0.01f);
    layout->addWidget(_l1_changer->slider, 6, 1, 1, 2);
    layout->addWidget(_l1_changer->spinbox, 6, 3);

    QLabel* s1_label = new QLabel("Saturation at end:");
    layout->addWidget(s1_label, 7, 0);
    _s1_changer = new ColorMapCombinedSliderSpinBox(0.0f, 1.0f, 0.01f);
    layout->addWidget(_s1_changer->slider, 7, 1, 1, 2);
    layout->addWidget(_s1_changer->spinbox, 7, 3);

    QLabel* s05_label = new QLabel("Saturation in the middle:");
    layout->addWidget(s05_label, 8, 0);
    _s05_changer = new ColorMapCombinedSliderSpinBox(0.0f, 1.0f, 0.01f);
    layout->addWidget(_s05_changer->slider, 8, 1, 1, 2);
    layout->addWidget(_s05_changer->spinbox, 8, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 9, 0, 1, 4);
    layout->setRowStretch(9, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_l0_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_s0_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_l1_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_s1_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_s05_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapPLSequentialMultiHueWidget::~ColorMapPLSequentialMultiHueWidget()
{
}

static QListWidgetItem* list_item_from_huepos(float hue, float pos)
{
    QLabel tmp("TMP");
    QPixmap pixmap(tmp.sizeHint().height(), tmp.sizeHint().height());
    pixmap.fill(hueToColor(hue));
    QListWidgetItem* lwi = new QListWidgetItem(QIcon(pixmap), QString::number(pos));
    return lwi;
}

void ColorMapPLSequentialMultiHueWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(256);
    _hue_list_widget->clear();
    for (int i = 0; i < ColorMap::PLSequentialMultiHueDefaultHues; i++) {
        _hue_list_widget->addItem(list_item_from_huepos(
                    ColorMap::PLSequentialMultiHueDefaultHueValues[i],
                    ColorMap::PLSequentialMultiHueDefaultHuePositions[i]));
    }
    _l0_changer->setValue(ColorMap::PLSequentialMultiHueDefaultL0 / 100.0f);
    _s0_changer->setValue(ColorMap::PLSequentialMultiHueDefaultS0);
    _l1_changer->setValue(ColorMap::PLSequentialMultiHueDefaultL1 / 100.0f);
    _s1_changer->setValue(ColorMap::PLSequentialMultiHueDefaultS1);
    _s05_changer->setValue(ColorMap::PLSequentialMultiHueDefaultS05);
    _update_lock = false;
    update();
}

QVector<unsigned char> ColorMapPLSequentialMultiHueWidget::colorMap(int* clipped) const
{
    int n;
    QVector<float> hue_values, hue_positions;
    float l0, s0, l1, s1, s05;
    parameters(n, hue_values, hue_positions, l0, s0, l1, s1, s05);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::PLSequentialMultiHue(n, colormap.data(),
            hue_values.size(), hue_values.data(), hue_positions.data(),
            l0, s0, l1, s1, s05);
    if (clipped)
        *clipped = cl;
    return colormap;
}

QString ColorMapPLSequentialMultiHueWidget::reference() const
{
    return plseq_multihue_reference;
}

void ColorMapPLSequentialMultiHueWidget::parameters(int& n,
        QVector<float>& hue_values, QVector<float>& hue_positions,
        float& l0, float& s0, float& l1, float& s1, float& s05) const
{
    n = _n_spinbox->value();
    hue_values.clear();
    hue_positions.clear();
    for (int i = 0; i < _hue_list_widget->count(); i++) {
        QListWidgetItem* item = _hue_list_widget->item(i);
        float hue = colorToHue(item->icon().pixmap(1, 1).toImage().pixelColor(0, 0));
        float pos = item->text().toFloat();
        hue_values.append(hue);
        hue_positions.append(pos);
    }
    l0 = std::max(0.01f, _l0_changer->value() * 100.0f);
    s0 = _s0_changer->value();
    l1 = std::max(0.01f, _l1_changer->value() * 100.0f);
    s1 = _s1_changer->value();
    s05 = _s05_changer->value();
}

void ColorMapPLSequentialMultiHueWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

void ColorMapPLSequentialMultiHueWidget::hueButtonClicked()
{
    QColor color = QColorDialog::getColor(_hue_button_color, this);
    if (color.isValid()) {
        _hue_button_color = hueToColor(colorToHue(color));
        updateHueButton();
    }
}

void ColorMapPLSequentialMultiHueWidget::updateHueButton()
{
    QPixmap pm(64, 64);
    pm.fill(_hue_button_color);
    _hue_button->setIcon(QIcon(pm));
}

void ColorMapPLSequentialMultiHueWidget::addHue()
{
    float hue = colorToHue(_hue_button_color);
    float pos = _pos_spinbox->value();
    for (int i = 0; i < _hue_list_widget->count(); i++) {
        QListWidgetItem* item = _hue_list_widget->item(i);
        float itemPos = item->text().toFloat();
        if (itemPos == pos) {
            delete _hue_list_widget->item(i);
            break;
        }
    }
    _hue_list_widget->insertItem(0, list_item_from_huepos(hue, pos));
    _hue_list_widget->sortItems();
    update();
}

void ColorMapPLSequentialMultiHueWidget::removeHue()
{
    delete _hue_list_widget->currentItem();
    update();
}

/* ColorMapPLDivergingLightnessWidget */

ColorMapPLDivergingLightnessWidget::ColorMapPLDivergingLightnessWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* lightness_label = new QLabel("Lightness:");
    layout->addWidget(lightness_label, 2, 0);
    _lightness_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_lightness_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_lightness_changer->spinbox, 2, 3);

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
    connect(_lightness_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_divergence_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapPLDivergingLightnessWidget::~ColorMapPLDivergingLightnessWidget()
{
}

void ColorMapPLDivergingLightnessWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(257);
    _lightness_changer->setValue(ColorMap::PLDivergingLightnessDefaultLightness);
    _saturation_changer->setValue(ColorMap::PLDivergingLightnessDefaultSaturation);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::PLDivergingLightnessDefaultHue));
    _divergence_changer->setValue(qRadiansToDegrees(ColorMap::PLDivergingLightnessDefaultDivergence));
    _update_lock = false;
    update();
}

QVector<unsigned char> ColorMapPLDivergingLightnessWidget::colorMap(int* clipped) const
{
    int n;
    float l, s, h, d;
    parameters(n, l, s, h, d);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::PLDivergingLightness(n, colormap.data(), l, s, h, d);
    if (clipped)
        *clipped = cl;
    return colormap;
}

QString ColorMapPLDivergingLightnessWidget::reference() const
{
    return pldiv_lightness_reference;
}

void ColorMapPLDivergingLightnessWidget::parameters(int& n, float& lightness, float& saturation, float& hue, float& divergence) const
{
    n = _n_spinbox->value();
    lightness = _lightness_changer->value();
    saturation = _saturation_changer->value();
    hue = qDegreesToRadians(_hue_changer->value());
    divergence = qDegreesToRadians(_divergence_changer->value());
}

void ColorMapPLDivergingLightnessWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapPLDivergingSaturationWidget */

ColorMapPLDivergingSaturationWidget::ColorMapPLDivergingSaturationWidget() :
    _update_lock(false)
{
    QGridLayout *layout = new QGridLayout;

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    _n_spinbox = new QSpinBox();
    _n_spinbox->setRange(2, 1024);
    _n_spinbox->setSingleStep(1);
    layout->addWidget(_n_spinbox, 1, 1, 1, 3);

    QLabel* lightness_label = new QLabel("Lightness:");
    layout->addWidget(lightness_label, 2, 0);
    _lightness_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_lightness_changer->slider, 2, 1, 1, 2);
    layout->addWidget(_lightness_changer->spinbox, 2, 3);

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
    connect(_lightness_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_divergence_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapPLDivergingSaturationWidget::~ColorMapPLDivergingSaturationWidget()
{
}

void ColorMapPLDivergingSaturationWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(257);
    _lightness_changer->setValue(ColorMap::PLDivergingSaturationDefaultLightness);
    _saturation_changer->setValue(ColorMap::PLDivergingSaturationDefaultSaturation);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::PLDivergingSaturationDefaultHue));
    _divergence_changer->setValue(qRadiansToDegrees(ColorMap::PLDivergingSaturationDefaultDivergence));
    _update_lock = false;
    update();
}

QVector<unsigned char> ColorMapPLDivergingSaturationWidget::colorMap(int* clipped) const
{
    int n;
    float l, s, h, d;
    parameters(n, l, s, h, d);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::PLDivergingSaturation(n, colormap.data(), l, s, h, d);
    if (clipped)
        *clipped = cl;
    return colormap;
}

QString ColorMapPLDivergingSaturationWidget::reference() const
{
    return pldiv_saturation_reference;
}

void ColorMapPLDivergingSaturationWidget::parameters(int& n, float& lightness, float& saturation, float& hue, float& divergence) const
{
    n = _n_spinbox->value();
    lightness = _lightness_changer->value();
    saturation = _saturation_changer->value();
    hue = qDegreesToRadians(_hue_changer->value());
    divergence = qDegreesToRadians(_divergence_changer->value());
}

void ColorMapPLDivergingSaturationWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

/* ColorMapPLQualitativeHueWidget */

ColorMapPLQualitativeHueWidget::ColorMapPLQualitativeHueWidget() :
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

    QLabel* lightness_label = new QLabel("Lightness:");
    layout->addWidget(lightness_label, 4, 0);
    _lightness_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_lightness_changer->slider, 4, 1, 1, 2);
    layout->addWidget(_lightness_changer->spinbox, 4, 3);

    QLabel* saturation_label = new QLabel("Saturation:");
    layout->addWidget(saturation_label, 5, 0);
    _saturation_changer = new ColorMapCombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(_saturation_changer->slider, 5, 1, 1, 2);
    layout->addWidget(_saturation_changer->spinbox, 5, 3);

    layout->setColumnStretch(1, 1);
    layout->addItem(new QSpacerItem(0, 0), 6, 0, 1, 4);
    layout->setRowStretch(6, 1);
    setLayout(layout);

    connect(_n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(_lightness_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(_divergence_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    reset();
}

ColorMapPLQualitativeHueWidget::~ColorMapPLQualitativeHueWidget()
{
}

void ColorMapPLQualitativeHueWidget::reset()
{
    _update_lock = true;
    _n_spinbox->setValue(256);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::PLQualitativeHueDefaultHue));
    _divergence_changer->setValue(qRadiansToDegrees(ColorMap::PLQualitativeHueDefaultDivergence));
    _lightness_changer->setValue(ColorMap::PLQualitativeHueDefaultLightness);
    _saturation_changer->setValue(ColorMap::PLQualitativeHueDefaultSaturation);
    _update_lock = false;
    update();
}

QVector<unsigned char> ColorMapPLQualitativeHueWidget::colorMap(int* clipped) const
{
    int n;
    float h, d, l, s;
    parameters(n, h, d, l, s);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::PLQualitativeHue(n, colormap.data(), h, d, l, s);
    if (clipped)
        *clipped = cl;
    return colormap;
}

QString ColorMapPLQualitativeHueWidget::reference() const
{
    return plqual_hue_reference;
}

void ColorMapPLQualitativeHueWidget::parameters(int& n, float& hue, float& divergence, float& lightness, float& saturation) const
{
    n = _n_spinbox->value();
    hue = qDegreesToRadians(_hue_changer->value());
    divergence = qDegreesToRadians(_divergence_changer->value());
    lightness = _lightness_changer->value();
    saturation = _saturation_changer->value();
}

void ColorMapPLQualitativeHueWidget::update()
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

QVector<unsigned char> ColorMapCubeHelixWidget::colorMap(int* clipped) const
{
    int n;
    float h, r, s, g;
    parameters(n, h, r, s, g);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::CubeHelix(n, colormap.data(), h, r, s, g);
    if (clipped)
        *clipped = cl;
    return colormap;
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

QVector<unsigned char> ColorMapMorelandWidget::colorMap(int* clipped) const
{
    int n;
    unsigned char r0, g0, b0, r1, g1, b1;
    parameters(n, r0, g0, b0, r1, g1, b1);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::Moreland(n, colormap.data(), r0, g0, b0, r1, g1, b1);
    if (clipped)
        *clipped = cl;
    return colormap;
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

QVector<unsigned char> ColorMapMcNamesWidget::colorMap(int* clipped) const
{
    int n;
    float p;
    parameters(n, p);
    QVector<unsigned char> colormap(3 * n);
    int cl = ColorMap::McNames(n, colormap.data(), p);
    if (clipped)
        *clipped = cl;
    return colormap;
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
