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

#ifndef COLORMAPWIDGETS_HPP
#define COLORMAPWIDGETS_HPP

#include <QVector>
#include <QWidget>

class QSpinBox;
class QSlider;
class QDoubleSpinBox;
class QPushButton;


// Internal helper class for a slider/spinbox combination
class ColorMapCombinedSliderSpinBox : public QObject
{
Q_OBJECT

private:
    bool _update_lock;

public:
    float minval, maxval, step;
    QSlider* slider;
    QDoubleSpinBox* spinbox;

    ColorMapCombinedSliderSpinBox(float minval, float maxval, float step);
    float value() const;
    void setValue(float v);

private slots:
    void sliderChanged();
    void spinboxChanged();

signals:
    void valueChanged(float);
};

/* The ColorMapWidget interface, implemented by all color map method widgets */
class ColorMapWidget : public QWidget
{
Q_OBJECT

public:
    ColorMapWidget();
    ~ColorMapWidget();

    /* Reset all values to their method-specific defaults */
    virtual void reset() = 0;

    /* Get the color map corresponding to the current values as a vector of colors */
    virtual QVector<QColor> colorMap() const = 0;

    /* Get a color map as an image of the specified size. If width or height is zero,
     * then it will be set to the number of colors in the color map. */
    QImage colorMapImage(int width, int height);

    /* Get a rich text string containing the relevant literature reference for this method */
    virtual QString reference() const = 0;

signals:
    void colorMapChanged();
};

class ColorMapBrewerSequentialWidget : public ColorMapWidget
{
Q_OBJECT
private:
    bool _update_lock;
    QSpinBox* _n_spinbox;
    ColorMapCombinedSliderSpinBox* _hue_changer;
    ColorMapCombinedSliderSpinBox* _warmth_changer;
    ColorMapCombinedSliderSpinBox* _contrast_changer;
    ColorMapCombinedSliderSpinBox* _saturation_changer;
    ColorMapCombinedSliderSpinBox* _brightness_changer;
private slots:
    void update();

public:
    ColorMapBrewerSequentialWidget();
    ~ColorMapBrewerSequentialWidget();

    void reset() override;
    QVector<QColor> colorMap() const override;
    QString reference() const override;
    void parameters(int& n, float& hue,
            float& contrast, float& saturation, float& brightness, float& warmth) const;
};

class ColorMapBrewerDivergingWidget : public ColorMapWidget
{
Q_OBJECT
private:
    bool _update_lock;
    QSpinBox* _n_spinbox;
    ColorMapCombinedSliderSpinBox* _hue_changer;
    ColorMapCombinedSliderSpinBox* _divergence_changer;
    ColorMapCombinedSliderSpinBox* _warmth_changer;
    ColorMapCombinedSliderSpinBox* _contrast_changer;
    ColorMapCombinedSliderSpinBox* _saturation_changer;
    ColorMapCombinedSliderSpinBox* _brightness_changer;
private slots:
    void update();

public:
    ColorMapBrewerDivergingWidget();
    ~ColorMapBrewerDivergingWidget();

    void reset() override;
    QVector<QColor> colorMap() const override;
    QString reference() const override;
    void parameters(int& n, float& hue, float& divergence,
            float& contrast, float& saturation, float& brightness, float& warmth) const;
};

class ColorMapBrewerQualitativeWidget : public ColorMapWidget
{
Q_OBJECT
private:
    bool _update_lock;
    QSpinBox* _n_spinbox;
    ColorMapCombinedSliderSpinBox* _hue_changer;
    ColorMapCombinedSliderSpinBox* _divergence_changer;
    ColorMapCombinedSliderSpinBox* _contrast_changer;
    ColorMapCombinedSliderSpinBox* _saturation_changer;
    ColorMapCombinedSliderSpinBox* _brightness_changer;
private slots:
    void update();

public:
    ColorMapBrewerQualitativeWidget();
    ~ColorMapBrewerQualitativeWidget();

    void reset() override;
    QVector<QColor> colorMap() const override;
    QString reference() const override;
    void parameters(int& n, float& hue, float& divergence,
            float& contrast, float& saturation, float& brightness) const;
};

class ColorMapIsoluminantSequentialWidget : public ColorMapWidget
{
Q_OBJECT
private:
    bool _update_lock;
    QSpinBox* _n_spinbox;
    ColorMapCombinedSliderSpinBox* _luminance_changer;
    ColorMapCombinedSliderSpinBox* _saturation_changer;
    ColorMapCombinedSliderSpinBox* _hue_changer;
private slots:
    void update();

public:
    ColorMapIsoluminantSequentialWidget();
    ~ColorMapIsoluminantSequentialWidget();

    void reset() override;
    QVector<QColor> colorMap() const override;
    QString reference() const override;
    void parameters(int& n, float& luminance, float& saturation, float& hue) const;
};

class ColorMapIsoluminantDivergingWidget : public ColorMapWidget
{
Q_OBJECT
private:
    bool _update_lock;
    QSpinBox* _n_spinbox;
    ColorMapCombinedSliderSpinBox* _luminance_changer;
    ColorMapCombinedSliderSpinBox* _saturation_changer;
    ColorMapCombinedSliderSpinBox* _hue_changer;
    ColorMapCombinedSliderSpinBox* _divergence_changer;
private slots:
    void update();

public:
    ColorMapIsoluminantDivergingWidget();
    ~ColorMapIsoluminantDivergingWidget();

    void reset() override;
    QVector<QColor> colorMap() const override;
    QString reference() const override;
    void parameters(int& n, float& luminance, float& saturation, float& hue, float& divergence) const;
};

class ColorMapIsoluminantQualitativeWidget : public ColorMapWidget
{
Q_OBJECT
private:
    bool _update_lock;
    QSpinBox* _n_spinbox;
    ColorMapCombinedSliderSpinBox* _luminance_changer;
    ColorMapCombinedSliderSpinBox* _saturation_changer;
    ColorMapCombinedSliderSpinBox* _hue_changer;
    ColorMapCombinedSliderSpinBox* _divergence_changer;
private slots:
    void update();

public:
    ColorMapIsoluminantQualitativeWidget();
    ~ColorMapIsoluminantQualitativeWidget();

    void reset() override;
    QVector<QColor> colorMap() const override;
    QString reference() const override;
    void parameters(int& n, float& luminance, float& saturation, float& hue, float& divergence) const;
};

class ColorMapCubeHelixWidget : public ColorMapWidget
{
Q_OBJECT
private:
    bool _update_lock;
    QSpinBox* _n_spinbox;
    ColorMapCombinedSliderSpinBox* _hue_changer;
    ColorMapCombinedSliderSpinBox* _rotations_changer;
    ColorMapCombinedSliderSpinBox* _saturation_changer;
    ColorMapCombinedSliderSpinBox* _gamma_changer;
private slots:
    void update();

public:
    ColorMapCubeHelixWidget();
    ~ColorMapCubeHelixWidget();

    void reset() override;
    QVector<QColor> colorMap() const override;
    QString reference() const override;
    void parameters(int& n, float& hue, float& rotations,
            float& saturation, float& gamma) const;
};

class ColorMapMorelandWidget : public ColorMapWidget
{
Q_OBJECT
private:
    bool _update_lock;
    QSpinBox* _n_spinbox;
    QPushButton* _color0_button;
    QPushButton* _color1_button;
private slots:
    void chooseColor0();
    void chooseColor1();
    void update();

public:
    ColorMapMorelandWidget();
    ~ColorMapMorelandWidget();

    void reset() override;
    QVector<QColor> colorMap() const override;
    QString reference() const override;
    void parameters(int& n,
            unsigned char& r0, unsigned char& g0, unsigned char& b0,
            unsigned char& r1, unsigned char& g1, unsigned char& b1) const;
};

class ColorMapMcNamesWidget : public ColorMapWidget
{
Q_OBJECT
private:
    bool _update_lock;
    QSpinBox* _n_spinbox;
    ColorMapCombinedSliderSpinBox* _periods_changer;
private slots:
    void update();

public:
    ColorMapMcNamesWidget();
    ~ColorMapMcNamesWidget();

    void reset() override;
    QVector<QColor> colorMap() const override;
    QString reference() const override;
    void parameters(int& n, float& p) const;
};

#endif
