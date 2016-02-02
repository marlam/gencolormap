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

#ifndef GUI_HPP
#define GUI_HPP

#include <QVector>
#include <QMainWindow>

class QTabWidget;
class QLabel;
class QRadioButton;
class QSpinBox;
class QSlider;
class QDoubleSpinBox;
class QTabWidget;


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

class ColorMapWidget : public QWidget
{
Q_OBJECT
protected:
    QVector<unsigned char> _colormap;
    virtual void recomputeColorMap() = 0;

public:
    ColorMapWidget() {}
    ~ColorMapWidget() {}

    virtual void reset() = 0;

    const QVector<unsigned char>* colorMap()
    {
        recomputeColorMap();
        return &_colormap;
    }

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

protected:
    void recomputeColorMap() override;

public:
    ColorMapBrewerSequentialWidget();
    ~ColorMapBrewerSequentialWidget();

    void reset() override;
    void parameters(int& n, float& hue,
            float& contrast, float& saturation, float& brightness, float& warmth);
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

protected:
    void recomputeColorMap() override;

public:
    ColorMapBrewerDivergingWidget();
    ~ColorMapBrewerDivergingWidget();

    void reset() override;
    void parameters(int& n, float& hue, float& divergence,
            float& contrast, float& saturation, float& brightness, float& warmth);
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

protected:
    void recomputeColorMap() override;

public:
    ColorMapBrewerQualitativeWidget();
    ~ColorMapBrewerQualitativeWidget();

    void reset() override;
    void parameters(int& n, float& hue, float& divergence,
            float& contrast, float& saturation, float& brightness);
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

protected:
    void recomputeColorMap() override;

public:
    ColorMapCubeHelixWidget();
    ~ColorMapCubeHelixWidget();

    void reset() override;
    void parameters(int& n, float& hue, float& rotations,
            float& saturation, float& gamma);
};

class GUI : public QMainWindow
{
Q_OBJECT

public:
    GUI();
    ~GUI();

private:
    ColorMapBrewerSequentialWidget* _brewerseq_widget;
    ColorMapBrewerDivergingWidget* _brewerdiv_widget;
    ColorMapBrewerQualitativeWidget* _brewerqual_widget;
    ColorMapCubeHelixWidget* _cubehelix_widget;
    QTabWidget* _tab_widget;
    QLabel* _colormap_label;

private slots:
    void update();

    void file_export_png();
    void file_export_csv();
    void edit_reset();
    void edit_copy_as_img();
    void edit_copy_as_txt();
    void help_about();
};

#endif
