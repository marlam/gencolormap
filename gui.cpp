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

#include <QApplication>
#include <QGuiApplication>
#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QMenu>
#include <QMenuBar>
#include <QImage>
#include <QPixmap>
#include <QFileDialog>
#include <QClipboard>
#include <QTextStream>
#include <QMessageBox>
#include <QtMath>

#include "colormap.hpp"


static QString brewerlike_reference = QString("Relevant paper: "
        "M. Wijffelaars, R. Vliegen, J. J. van Wijk, E-J. van der Linden, "
        "<a href=\"http://dx.doi.org/10.1111/j.1467-8659.2008.01203.x\">Generating Color Palettes using Intuitive Parameters</a>, "
        "Computer Graphics Forum 27(3), May 2008.");

static QString cubehelix_reference = QString("Relevant paper: "
        "D. A. Green, "
        "<a href=\"http://www.mrao.cam.ac.uk/~dag/CUBEHELIX/\">A colour scheme for the display of astronomical intensity</a>, "
        "Bulletin of the Astronomical Society of India 39(2), June 2011.");


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

static void hideWidgetButPreserveSize(QWidget* widget)
{
    QSizePolicy sp = widget->sizePolicy();
    sp.setRetainSizeWhenHidden(true);
    widget->setSizePolicy(sp);
    widget->hide();
}

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

    layout->addItem(new QSpacerItem(0, 0), 8, 0, 1, 4);
    QLabel* reference_label = new QLabel(brewerlike_reference);
    reference_label->setWordWrap(true);
    reference_label->setOpenExternalLinks(true);
    layout->addWidget(reference_label, 9, 0, 1, 4);

    layout->setColumnStretch(1, 1);
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

void ColorMapBrewerSequentialWidget::parameters(int& n, float& hue,
        float& contrast, float& saturation, float& brightness, float& warmth)
{
    n = _n_spinbox->value();
    hue = qDegreesToRadians(_hue_changer->value());
    contrast = _contrast_changer->value();
    saturation = _saturation_changer->value();
    brightness = _brightness_changer->value();
    warmth = _warmth_changer->value();
}

void ColorMapBrewerSequentialWidget::recomputeColorMap()
{
    int n;
    float h, c, s, b, w;
    parameters(n, h, c, s, b, w);
    _colormap.resize(3 * n);
    ColorMap::BrewerSequential(n, _colormap.data(), h, c, s, b, w);
}

void ColorMapBrewerSequentialWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

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

    layout->addItem(new QSpacerItem(0, 0), 8, 0, 1, 4);
    QLabel* reference_label = new QLabel(brewerlike_reference);
    reference_label->setWordWrap(true);
    reference_label->setOpenExternalLinks(true);
    layout->addWidget(reference_label, 9, 0, 1, 4);

    layout->setColumnStretch(1, 1);
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
    _n_spinbox->setValue(256);
    _hue_changer->setValue(qRadiansToDegrees(ColorMap::BrewerDivergingDefaultHue));
    _divergence_changer->setValue(qRadiansToDegrees(ColorMap::BrewerDivergingDefaultDivergence));
    _warmth_changer->setValue(ColorMap::BrewerDivergingDefaultWarmth);
    _contrast_changer->setValue(ColorMap::BrewerDivergingDefaultContrast);
    _saturation_changer->setValue(ColorMap::BrewerDivergingDefaultSaturation);
    _brightness_changer->setValue(ColorMap::BrewerDivergingDefaultBrightness);
    _update_lock = false;
    update();
}

void ColorMapBrewerDivergingWidget::parameters(int& n, float& hue, float& divergence,
        float& contrast, float& saturation, float& brightness, float& warmth)
{
    n = _n_spinbox->value();
    hue = qDegreesToRadians(_hue_changer->value());
    divergence = qDegreesToRadians(_divergence_changer->value());
    contrast = _contrast_changer->value();
    saturation = _saturation_changer->value();
    brightness = _brightness_changer->value();
    warmth = _warmth_changer->value();
}

void ColorMapBrewerDivergingWidget::recomputeColorMap()
{
    int n;
    float h, d, c, s, b, w;
    parameters(n, h, d, c, s, b, w);
    _colormap.resize(3 * n);
    ColorMap::BrewerDiverging(n, _colormap.data(), h, d, c, s, b, w);
}

void ColorMapBrewerDivergingWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

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

    layout->addItem(new QSpacerItem(0, 0), 8, 0, 1, 4);
    QLabel* reference_label = new QLabel(brewerlike_reference);
    reference_label->setWordWrap(true);
    reference_label->setOpenExternalLinks(true);
    layout->addWidget(reference_label, 9, 0, 1, 4);

    layout->setColumnStretch(1, 1);
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

void ColorMapBrewerQualitativeWidget::parameters(int& n, float& hue, float& divergence,
        float& contrast, float& saturation, float& brightness)
{
    n = _n_spinbox->value();
    hue = qDegreesToRadians(_hue_changer->value());
    divergence = qDegreesToRadians(_divergence_changer->value());
    contrast = _contrast_changer->value();
    saturation = _saturation_changer->value();
    brightness = _brightness_changer->value();
}

void ColorMapBrewerQualitativeWidget::recomputeColorMap()
{
    int n;
    float h, d, c, s, b;
    parameters(n, h, d, c, s, b);
    _colormap.resize(3 * n);
    ColorMap::BrewerQualitative(n, _colormap.data(), h, d, c, s, b);
}

void ColorMapBrewerQualitativeWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}

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

    layout->addItem(new QSpacerItem(0, 0), 6, 0, 1, 4);
    QLabel* reference_label = new QLabel(cubehelix_reference);
    reference_label->setWordWrap(true);
    reference_label->setOpenExternalLinks(true);
    layout->addWidget(reference_label, 7, 0, 1, 4);

    layout->setColumnStretch(1, 1);
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

void ColorMapCubeHelixWidget::parameters(int& n, float& hue,
        float& rotations, float& saturation, float& gamma)
{
    n = _n_spinbox->value();
    hue = qDegreesToRadians(_hue_changer->value());
    rotations = _rotations_changer->value();
    saturation = _saturation_changer->value();
    gamma = _gamma_changer->value();
}

void ColorMapCubeHelixWidget::recomputeColorMap()
{
    int n;
    float h, r, s, g;
    parameters(n, h, r, s, g);
    _colormap.resize(3 * n);
    ColorMap::CubeHelix(n, _colormap.data(), h, r, s, g);
}

void ColorMapCubeHelixWidget::update()
{
    if (!_update_lock)
        emit colorMapChanged();
}


GUI::GUI()
{
    setWindowTitle("Generate Color Map");
    setWindowIcon(QIcon(":cg-logo.png"));

    _brewerseq_widget = new ColorMapBrewerSequentialWidget;
    _brewerdiv_widget = new ColorMapBrewerDivergingWidget;
    _brewerqual_widget = new ColorMapBrewerQualitativeWidget;
    _cubehelix_widget = new ColorMapCubeHelixWidget;
    connect(_brewerseq_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_brewerdiv_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_brewerqual_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_cubehelix_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));

    QWidget *widget = new QWidget;
    widget->setMinimumWidth(384 * qApp->devicePixelRatio());
    QGridLayout *layout = new QGridLayout;

    _tab_widget = new QTabWidget();
    _tab_widget->addTab(_brewerseq_widget,  "Brewer-like Sequential");
    _tab_widget->addTab(_brewerdiv_widget,  "Brewer-like Diverging");
    _tab_widget->addTab(_brewerqual_widget, "Brewer-like Qualitative");
    _tab_widget->addTab(_cubehelix_widget,  "CubeHelix");
    connect(_tab_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    layout->addWidget(_tab_widget, 0, 0);

    _colormap_label = new QLabel();
    _colormap_label->setScaledContents(true);
    layout->addWidget(_colormap_label, 0, 1);

    layout->setColumnStretch(0, 1);
    widget->setLayout(layout);
    setCentralWidget(widget);

    QMenu* file_menu = menuBar()->addMenu("&File");
    QAction* file_export_png_act = new QAction("Export as &PNG...", this);
    connect(file_export_png_act, SIGNAL(triggered()), this, SLOT(file_export_png()));
    file_menu->addAction(file_export_png_act);
    QAction* file_export_csv_act = new QAction("Export as &CSV...", this);
    connect(file_export_csv_act, SIGNAL(triggered()), this, SLOT(file_export_csv()));
    file_menu->addAction(file_export_csv_act);
    file_menu->addSeparator();
    QAction* quit_act = new QAction("&Quit...", this);
    quit_act->setShortcut(QKeySequence::Quit);
    connect(quit_act, SIGNAL(triggered()), this, SLOT(close()));
    file_menu->addAction(quit_act);
    QMenu* edit_menu = menuBar()->addMenu("&Edit");
    QAction* edit_reset_act = new QAction("&Reset", this);
    connect(edit_reset_act, SIGNAL(triggered()), this, SLOT(edit_reset()));
    edit_menu->addAction(edit_reset_act);
    QAction* edit_copy_as_img_act = new QAction("Copy as &image", this);
    connect(edit_copy_as_img_act, SIGNAL(triggered()), this, SLOT(edit_copy_as_img()));
    edit_menu->addAction(edit_copy_as_img_act);
    QAction* edit_copy_as_txt_act = new QAction("Copy as &text", this);
    connect(edit_copy_as_txt_act, SIGNAL(triggered()), this, SLOT(edit_copy_as_txt()));
    edit_copy_as_txt_act->setShortcut(QKeySequence::Copy);
    edit_menu->addAction(edit_copy_as_txt_act);
    QMenu* help_menu = menuBar()->addMenu("&Help");
    QAction* help_about_act = new QAction("&About", this);
    connect(help_about_act, SIGNAL(triggered()), this, SLOT(help_about()));
    help_menu->addAction(help_about_act);

    show();
    update();
}

GUI::~GUI()
{
}

void GUI::update()
{
    ColorMapWidget* currentWidget = reinterpret_cast<ColorMapWidget*>(_tab_widget->currentWidget());
    const QVector<unsigned char>& colormap = *currentWidget->colorMap();

    int img_width = 32;
    int img_height = _colormap_label->height();
    QImage img(img_width, img_height, QImage::Format_RGB32);
    for (int y = 0; y < img_height; y++) {
        float entry_height = img_height / static_cast<float>(colormap.size() / 3);
        int i = y / entry_height;
        QRgb rgb = QColor(colormap[3 * i + 0], colormap[3 * i + 1], colormap[3 * i + 2]).rgb();
        QRgb* scanline = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img_width; x++)
            scanline[x] = rgb;
    }
    _colormap_label->setPixmap(QPixmap::fromImage(img));
}

void GUI::file_export_png()
{
    QString name = QFileDialog::getSaveFileName();
    if (!name.isEmpty()) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        ColorMapWidget* currentWidget = reinterpret_cast<ColorMapWidget*>(_tab_widget->currentWidget());
        const QVector<unsigned char>& colormap = *currentWidget->colorMap();
        QImage img(colormap.size() / 3, 1, QImage::Format_RGB32);
        QRgb* scanline = reinterpret_cast<QRgb*>(img.scanLine(0));
        for (int i = 0; i < colormap.size() / 3; i++) {
            scanline[i] = QColor(colormap[3 * i + 0], colormap[3 * i + 1], colormap[3 * i + 2]).rgb();
        }
        img.save(name, "png");
        QApplication::restoreOverrideCursor();
    }
}

void GUI::file_export_csv()
{
    QString name = QFileDialog::getSaveFileName();
    if (!name.isEmpty()) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QFile file(name);
        if (file.open(QIODevice::ReadWrite)) {
            ColorMapWidget* currentWidget = reinterpret_cast<ColorMapWidget*>(_tab_widget->currentWidget());
            const QVector<unsigned char>& colormap = *currentWidget->colorMap();
            QTextStream stream(&file);
            for (int i = 0; i < colormap.size() / 3; i++) {
                stream << colormap[3 * i + 0] << ", "
                       << colormap[3 * i + 1] << ", "
                       << colormap[3 * i + 2] << endl;
            }
        }
        QApplication::restoreOverrideCursor();
    }
}

void GUI::edit_reset()
{
    ColorMapWidget* currentWidget = reinterpret_cast<ColorMapWidget*>(_tab_widget->currentWidget());
    currentWidget->reset();
}

void GUI::edit_copy_as_img()
{
    ColorMapWidget* currentWidget = reinterpret_cast<ColorMapWidget*>(_tab_widget->currentWidget());
    const QVector<unsigned char>& colormap = *currentWidget->colorMap();
    QImage img(colormap.size() / 3, 1, QImage::Format_RGB32);
    QRgb* scanline = reinterpret_cast<QRgb*>(img.scanLine(0));
    for (int i = 0; i < colormap.size() / 3; i++) {
        scanline[i] = QColor(colormap[3 * i + 0], colormap[3 * i + 1], colormap[3 * i + 2]).rgb();
    }
    QApplication::clipboard()->setImage(img);
}

void GUI::edit_copy_as_txt()
{
    ColorMapWidget* currentWidget = reinterpret_cast<ColorMapWidget*>(_tab_widget->currentWidget());
    const QVector<unsigned char>& colormap = *currentWidget->colorMap();
    QString string;
    QTextStream stream(&string);
    for (int i = 0; i < colormap.size() / 3; i++) {
        stream << colormap[3 * i + 0] << ", "
               << colormap[3 * i + 1] << ", "
               << colormap[3 * i + 2] << endl;
    }
    QApplication::clipboard()->setText(string);
}

void GUI::help_about()
{
    QMessageBox::about(this, "About",
                "<p>gencolormap version 0.2</p>"
                "<p>Copyright (C) 2016<br>"
                "   <a href=\"http://www.cg.informatik.uni-siegen.de/\">"
                "   Computer Graphics Group, University of Siegen</a>.<br>"
                "   Written by <a href=\"http://www.cg.informatik.uni-siegen.de/lambers-martin\">Martin Lambers</a>.<br>"
                "   This is free software under the terms of the "
                    "<a href=\"https://www.debian.org/legal/licenses/mit\">MIT/Expat License</a>. "
                "   There is NO WARRANTY, to the extent permitted by law."
                "</p>"
                "<p>This program implements the color map generation techniques described in<br>"
                "   M. Wijffelaars, R. Vliegen, J.J. van Wijk, E.-J. van der Linden."
                "   Generating color palettes using intuitive parameters. "
                "   In Computer Graphics Forum, vol. 27, no. 3, pp. 743-750, 2008."
                "</p>");
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    GUI gui;
    gui.show();
    return app.exec();
}
