/*
 * Copyright (C) 2015, 2016, 2020
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

#ifndef GUI_HPP
#define GUI_HPP

#include <QMainWindow>

class ColorMapWidget;
class ColorMapBrewerSequentialWidget;
class ColorMapBrewerDivergingWidget;
class ColorMapBrewerQualitativeWidget;
class ColorMapPUSequentialLightnessWidget;
class ColorMapPUSequentialSaturationWidget;
class ColorMapPUSequentialRainbowWidget;
class ColorMapPUSequentialBlackBodyWidget;
class ColorMapPUSequentialMultiHueWidget;
class ColorMapPUDivergingLightnessWidget;
class ColorMapPUDivergingSaturationWidget;
class ColorMapPUQualitativeHueWidget;
class ColorMapCubeHelixWidget;
class ColorMapMorelandWidget;
class ColorMapMcNamesWidget;
class ColorMapTestWidget;
class QTabWidget;
class QLabel;
class QRadioButton;


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
    ColorMapPUSequentialLightnessWidget *_puseq_lightness_widget;
    ColorMapPUSequentialSaturationWidget *_puseq_saturation_widget;
    ColorMapPUSequentialRainbowWidget *_puseq_rainbow_widget;
    ColorMapPUSequentialBlackBodyWidget *_puseq_blackbody_widget;
    ColorMapPUSequentialMultiHueWidget *_puseq_multihue_widget;
    ColorMapPUDivergingLightnessWidget *_pudiv_lightness_widget;
    ColorMapPUDivergingSaturationWidget *_pudiv_saturation_widget;
    ColorMapPUQualitativeHueWidget *_puqual_hue_widget;
    ColorMapCubeHelixWidget* _cubehelix_widget;
    ColorMapMorelandWidget* _moreland_widget;
    ColorMapMcNamesWidget* _mcnames_widget;
    QTabWidget* _category_widget;
    QTabWidget* _category_seq_widget;
    QTabWidget* _category_div_widget;
    QTabWidget* _category_qual_widget;
    QLabel* _reference_label;
    QLabel* _clipped_label;
    QLabel* _colormap_label;
    ColorMapTestWidget* _test_widget;
    QRadioButton* _export_format_png_button;
    QRadioButton* _export_format_ppm_button;
    QRadioButton* _export_format_csv_button;
    QRadioButton* _export_format_json_button;

    ColorMapWidget* currentWidget();

private slots:
    void update();

    void file_export();
    void edit_reset();
    void edit_copy();
    void help_about();
};

#endif
