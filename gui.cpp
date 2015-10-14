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

#include <cmath>

#include "gui.hpp"

#include <QApplication>
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

#include "colormap.hpp"


CombinedSliderSpinBox::CombinedSliderSpinBox(float minval, float maxval, float step) :
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

    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(slider_changed()));
    connect(spinbox, SIGNAL(valueChanged(double)), this, SLOT(spinbox_changed()));
}

float CombinedSliderSpinBox::value() const
{
    return spinbox->value();
}

void CombinedSliderSpinBox::setValue(float v)
{
    _update_lock = true;
    spinbox->setValue(v);
    slider->setValue((v - minval) / step);
    _update_lock = false;
}

void CombinedSliderSpinBox::slider_changed()
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

void CombinedSliderSpinBox::spinbox_changed()
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

GUI::GUI()
{
    setWindowTitle("Generate Color Map");
    setWindowIcon(QIcon(":cg-logo.png"));
    QWidget *widget = new QWidget;
    QGridLayout *layout = new QGridLayout;

    QLabel* type_label = new QLabel("Type:");
    layout->addWidget(type_label, 0, 0);
    type_seq_btn = new QRadioButton("Sequential");
    layout->addWidget(type_seq_btn, 0, 1);
    type_div_btn = new QRadioButton("Diverging");
    layout->addWidget(type_div_btn, 0, 2);
    QRadioButton* type_qual_btn = new QRadioButton("Qualitative");
    layout->addWidget(type_qual_btn, 0, 3);

    QLabel* n_label = new QLabel("Colors:");
    layout->addWidget(n_label, 1, 0);
    n_spinbox = new QSpinBox();
    n_spinbox->setRange(2, 1024);
    n_spinbox->setSingleStep(1);
    layout->addWidget(n_spinbox, 1, 1, 1, 3);

    QLabel* hue_label = new QLabel("Hue:");
    layout->addWidget(hue_label, 2, 0);
    hue_changer = new CombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(hue_changer->slider, 2, 1, 1, 2);
    layout->addWidget(hue_changer->spinbox, 2, 3);

    divergence_label = new QLabel("Divergence:");
    layout->addWidget(divergence_label, 3, 0);
    divergence_changer = new CombinedSliderSpinBox(0, 360, 1);
    layout->addWidget(divergence_changer->slider, 3, 1, 1, 2);
    layout->addWidget(divergence_changer->spinbox, 3, 3);

    warmth_label = new QLabel("Warmth:");
    layout->addWidget(warmth_label, 4, 0);
    warmth_changer = new CombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(warmth_changer->slider, 4, 1, 1, 2);
    layout->addWidget(warmth_changer->spinbox, 4, 3);

    QLabel* contrast_label = new QLabel("Contrast:");
    layout->addWidget(contrast_label, 5, 0);
    contrast_changer = new CombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(contrast_changer->slider, 5, 1, 1, 2);
    layout->addWidget(contrast_changer->spinbox, 5, 3);

    QLabel* saturation_label = new QLabel("Saturation:");
    layout->addWidget(saturation_label, 6, 0);
    saturation_changer = new CombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(saturation_changer->slider, 6, 1, 1, 2);
    layout->addWidget(saturation_changer->spinbox, 6, 3);

    QLabel* brightness_label = new QLabel("Brightness:");
    layout->addWidget(brightness_label, 7, 0);
    brightness_changer = new CombinedSliderSpinBox(0, 1, 0.01f);
    layout->addWidget(brightness_changer->slider, 7, 1, 1, 2);
    layout->addWidget(brightness_changer->spinbox, 7, 3);

    connect(type_seq_btn, SIGNAL(toggled(bool)), this, SLOT(update()));
    connect(n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(type_div_btn, SIGNAL(toggled(bool)), this, SLOT(update()));
    connect(hue_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(divergence_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(warmth_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(contrast_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(saturation_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));
    connect(brightness_changer, SIGNAL(valueChanged(float)), this, SLOT(update()));

    colormap_label = new QLabel();
    colormap_label->setScaledContents(true);
    layout->addWidget(colormap_label, 0, 4, 8, 1);

    layout->setColumnStretch(4, 1);
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
    edit_reset();
}

GUI::~GUI()
{
}

void GUI::get_params(int& type, int& n, float& hue, float& divergence,
        float& contrast, float& saturation, float& brightness,
        float& warmth)
{
    type = type_seq_btn->isChecked() ? 0 : type_div_btn->isChecked() ? 1 : 2;
    n = n_spinbox->value();
    hue = hue_changer->value() / 180.0f * static_cast<float>(M_PI);
    divergence = divergence_changer->value() / 180.0f * static_cast<float>(M_PI);
    contrast = contrast_changer->value();
    saturation = saturation_changer->value();
    brightness = brightness_changer->value();
    warmth = warmth_changer->value();
}

std::vector<unsigned char> GUI::get_map(int type, int n, float hue, float divergence,
        float contrast, float saturation, float brightness,
        float warmth)
{
    std::vector<unsigned char> colormap(3 * n);
    if (type == 0) {
        ColorMap::Sequential(n, &(colormap[0]), hue,
                contrast, saturation, brightness, warmth);
    } else if (type == 1) {
        ColorMap::Diverging(n, &(colormap[0]), hue, divergence,
                contrast, saturation, brightness, warmth);
    } else {
        ColorMap::Qualitative(n, &(colormap[0]), hue, divergence,
                contrast, saturation, brightness);
    }
    return colormap;
}

void GUI::update()
{
    if (update_lock)
        return;

    int type, n;
    float hue, divergence, contrast, saturation, brightness, warmth;
    get_params(type, n, hue, divergence, contrast, saturation, brightness, warmth);

    divergence_label->setEnabled(type >= 1);
    divergence_changer->slider->setEnabled(type >= 1);
    divergence_changer->spinbox->setEnabled(type >= 1);
    warmth_label->setEnabled(type <= 1);
    warmth_changer->slider->setEnabled(type <= 1);
    warmth_changer->spinbox->setEnabled(type <= 1);

    const int img_width = 32;
    const int img_height = colormap_label->height();
    if (img_height < n)
        n = img_height;
    QImage img(img_width, img_height, QImage::Format_RGB32);

    std::vector<unsigned char> colormap(3 * n);
    if (type == 0) {
        ColorMap::Sequential(n, &(colormap[0]), hue,
                contrast, saturation, brightness, warmth);
    } else if (type == 1) {
        ColorMap::Diverging(n, &(colormap[0]), hue, divergence,
                contrast, saturation, brightness, warmth);
    } else {
        ColorMap::Qualitative(n, &(colormap[0]), hue, divergence,
                contrast, saturation, brightness);
    }
    for (int y = 0; y < img_height; y++) {
        float entry_height = img_height / static_cast<float>(n);
        int i = y / entry_height;
        QRgb rgb = QColor(colormap[3 * i + 0], colormap[3 * i + 1], colormap[3 * i + 2]).rgb();
        QRgb* scanline = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img_width; x++)
            scanline[x] = rgb;
    }
    colormap_label->setPixmap(QPixmap::fromImage(img));
}

void GUI::file_export_png()
{
    QString name = QFileDialog::getSaveFileName();
    if (!name.isEmpty()) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        int type, n;
        float hue, divergence, contrast, saturation, brightness, warmth;
        get_params(type, n, hue, divergence, contrast, saturation, brightness, warmth);
        std::vector<unsigned char> colormap = get_map(type, n, hue, divergence,
                contrast, saturation, brightness, warmth);
        QImage img(n, 1, QImage::Format_RGB32);
        QRgb* scanline = reinterpret_cast<QRgb*>(img.scanLine(0));
        for (int i = 0; i < n; i++) {
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
            int type, n;
            float hue, divergence, contrast, saturation, brightness, warmth;
            get_params(type, n, hue, divergence, contrast, saturation, brightness, warmth);
            std::vector<unsigned char> colormap = get_map(type, n, hue, divergence,
                    contrast, saturation, brightness, warmth);
            QTextStream stream(&file);
            for (int i = 0; i < n; i++) {
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
    update_lock = true;
    type_seq_btn->setChecked(true);
    n_spinbox->setValue(301);
    hue_changer->setValue(0);
    divergence_changer->setValue(240);
    warmth_changer->setValue(ColorMap::DefaultWarmth);
    contrast_changer->setValue(ColorMap::DefaultContrast);
    saturation_changer->setValue(ColorMap::DefaultSaturation);
    brightness_changer->setValue(ColorMap::DefaultBrightness);
    update_lock = false;
    update();
}

void GUI::edit_copy_as_img()
{
    int type, n;
    float hue, divergence, contrast, saturation, brightness, warmth;
    get_params(type, n, hue, divergence, contrast, saturation, brightness, warmth);
    std::vector<unsigned char> colormap = get_map(type, n, hue, divergence,
            contrast, saturation, brightness, warmth);
    QImage img(n, 1, QImage::Format_RGB32);
    QRgb* scanline = reinterpret_cast<QRgb*>(img.scanLine(0));
    for (int i = 0; i < n; i++) {
        scanline[i] = QColor(colormap[3 * i + 0], colormap[3 * i + 1], colormap[3 * i + 2]).rgb();
    }
    QApplication::clipboard()->setImage(img);
}

void GUI::edit_copy_as_txt()
{
    int type, n;
    float hue, divergence, contrast, saturation, brightness, warmth;
    get_params(type, n, hue, divergence, contrast, saturation, brightness, warmth);
    std::vector<unsigned char> colormap = get_map(type, n, hue, divergence,
            contrast, saturation, brightness, warmth);
    QString string;
    QTextStream stream(&string);
    for (int i = 0; i < n; i++) {
        stream << colormap[3 * i + 0] << ", "
               << colormap[3 * i + 1] << ", "
               << colormap[3 * i + 2] << endl;
    }
    QApplication::clipboard()->setText(string);
}

void GUI::help_about()
{
    QMessageBox::about(this, "About",
                "<p>gencolormap version 0.1</p>"
                "<p>Copyright (C) 2015<br>"
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
