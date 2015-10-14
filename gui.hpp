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

#ifndef GUI_HPP
#define GUI_HPP

#include <vector>

#include <QMainWindow>

class QLabel;
class QRadioButton;
class QSpinBox;
class QSlider;
class QDoubleSpinBox;


class CombinedSliderSpinBox : public QObject
{
Q_OBJECT
private:
    bool _update_lock;
public:
    float minval, maxval, step;
    QSlider* slider;
    QDoubleSpinBox* spinbox;

    CombinedSliderSpinBox(float minval, float maxval, float step);
    float value() const;
    void setValue(float v);

private slots:
    void slider_changed();
    void spinbox_changed();

signals:
    void valueChanged(float);
};

class GUI : public QMainWindow
{
Q_OBJECT

public:
    GUI();
    ~GUI();

private:
    bool update_lock;
    QRadioButton* type_seq_btn;
    QRadioButton* type_div_btn;
    QSpinBox* n_spinbox;
    CombinedSliderSpinBox* hue_changer;
    QLabel* divergence_label;
    CombinedSliderSpinBox* divergence_changer;
    QLabel* warmth_label;
    CombinedSliderSpinBox* warmth_changer;
    CombinedSliderSpinBox* contrast_changer;
    CombinedSliderSpinBox* saturation_changer;
    CombinedSliderSpinBox* brightness_changer;
    QLabel* colormap_label;

    void get_params(int& type, int& n, float& hue, float& divergence,
            float& contrast, float& saturation, float& brightness,
            float& warmth);
    std::vector<unsigned char> get_map(int type, int n, float hue, float divergence,
            float contrast, float saturation, float brightness,
            float warmth);


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
