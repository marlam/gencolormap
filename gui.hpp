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

#include <QMainWindow>

class ColorMapWidget;
class ColorMapBrewerSequentialWidget;
class ColorMapBrewerDivergingWidget;
class ColorMapBrewerQualitativeWidget;
class ColorMapPLSequentialLightnessWidget;
class ColorMapPLSequentialSaturationWidget;
class ColorMapPLSequentialRainbowWidget;
class ColorMapPLSequentialBlackBodyWidget;
class ColorMapPLSequentialMultiHueWidget;
class ColorMapPLDivergingLightnessWidget;
class ColorMapPLDivergingSaturationWidget;
class ColorMapPLQualitativeHueWidget;
class ColorMapCubeHelixWidget;
class ColorMapMorelandWidget;
class ColorMapMcNamesWidget;
class ColorMapTestWidget;
class QTabWidget;
class QLabel;


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
    ColorMapPLSequentialLightnessWidget *_plseq_lightness_widget;
    ColorMapPLSequentialSaturationWidget *_plseq_saturation_widget;
    ColorMapPLSequentialRainbowWidget *_plseq_rainbow_widget;
    ColorMapPLSequentialBlackBodyWidget *_plseq_blackbody_widget;
    ColorMapPLSequentialMultiHueWidget *_plseq_multihue_widget;
    ColorMapPLDivergingLightnessWidget *_pldiv_lightness_widget;
    ColorMapPLDivergingSaturationWidget *_pldiv_saturation_widget;
    ColorMapPLQualitativeHueWidget *_plqual_hue_widget;
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

    ColorMapWidget* currentWidget();

private slots:
    void update();

    void file_export_png();
    void file_export_csv();
    void file_export_json();
    void edit_reset();
    void edit_copy_as_img();
    void edit_copy_as_csv();
    void edit_copy_as_json();
    void help_about();
};

#endif
