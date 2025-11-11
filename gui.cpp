/*
 * Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022
 * Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>
 * Copyright (C) 2022, 2023, 2024, 2025
 * Martin Lambers <marlam@marlam.de>
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
#include <QTabWidget>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QClipboard>
#include <QTextStream>
#include <QMessageBox>
#include <QButtonGroup>
#include <QByteArray>
#include <QBuffer>

#include "colormapwidgets.hpp"
#include "testwidget.hpp"
#include "export.hpp"


GUI::GUI()
{
    setWindowTitle("Generate Color Map");
    setWindowIcon(QIcon(":res/gencolormap-logo-512.png"));

    _brewerseq_widget = new ColorMapBrewerSequentialWidget;
    _brewerdiv_widget = new ColorMapBrewerDivergingWidget;
    _brewerqual_widget = new ColorMapBrewerQualitativeWidget;
    _puseq_lightness_widget = new ColorMapPUSequentialLightnessWidget;
    _puseq_saturation_widget = new ColorMapPUSequentialSaturationWidget;
    _puseq_rainbow_widget = new ColorMapPUSequentialRainbowWidget;
    _puseq_blackbody_widget = new ColorMapPUSequentialBlackBodyWidget;
    _puseq_multihue_widget = new ColorMapPUSequentialMultiHueWidget;
    _pudiv_lightness_widget = new ColorMapPUDivergingLightnessWidget;
    _pudiv_saturation_widget = new ColorMapPUDivergingSaturationWidget;
    _puqual_hue_widget = new ColorMapPUQualitativeHueWidget;
    _cubehelix_widget = new ColorMapCubeHelixWidget;
    _moreland_widget = new ColorMapMorelandWidget;
    _mcnames_widget = new ColorMapMcNamesWidget;
    connect(_brewerseq_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_brewerdiv_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_brewerqual_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_puseq_lightness_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_puseq_saturation_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_puseq_rainbow_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_puseq_blackbody_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_puseq_multihue_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_pudiv_lightness_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_pudiv_saturation_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_puqual_hue_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_cubehelix_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_moreland_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_mcnames_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));

    QWidget *widget = new QWidget;
    QGridLayout *layout = new QGridLayout;

    _category_widget = new QTabWidget();
    _category_seq_widget = new QTabWidget();
    _category_seq_widget->addTab(_puseq_multihue_widget, "PU Multi Hue");
    _category_seq_widget->addTab(_puseq_lightness_widget, "PU Lightness");
    _category_seq_widget->addTab(_puseq_saturation_widget, "PU Saturation");
    _category_seq_widget->addTab(_puseq_rainbow_widget, "PU Rainbow");
    _category_seq_widget->addTab(_puseq_blackbody_widget, "PU Black Body");
    _category_seq_widget->addTab(_brewerseq_widget, "Brewer-like");
    _category_seq_widget->addTab(_cubehelix_widget, "CubeHelix");
    //_category_seq_widget->addTab(_mcnames_widget, "McNames");
    connect(_category_seq_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    _category_widget->addTab(_category_seq_widget, "Sequential");
    _category_div_widget = new QTabWidget();
    _category_div_widget->addTab(_pudiv_lightness_widget, "PU Lightness");
    _category_div_widget->addTab(_pudiv_saturation_widget, "PU Saturation");
    _category_div_widget->addTab(_brewerdiv_widget, "Brewer-like");
    _category_div_widget->addTab(_moreland_widget, "Moreland");
    connect(_category_div_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    _category_widget->addTab(_category_div_widget, "Diverging");
    _category_qual_widget = new QTabWidget();
    _category_qual_widget->addTab(_puqual_hue_widget, "PU Hue");
    _category_qual_widget->addTab(_brewerqual_widget, "Brewer-like");
    connect(_category_qual_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    _category_widget->addTab(_category_qual_widget, "Qualitative");
    connect(_category_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    layout->addWidget(_category_widget, 0, 0);
    _reference_label = new QLabel(_brewerseq_widget->reference());
    _reference_label->setWordWrap(true);
    _reference_label->setOpenExternalLinks(true);
    layout->addWidget(_reference_label, 1, 0);
    _clipped_label = new QLabel("");
    layout->addWidget(_clipped_label, 2, 0);

    _colormap_label = new QLabel();
    _colormap_label->setScaledContents(true);
    layout->addWidget(_colormap_label, 0, 1, 4, 1);

    QLabel* test_label = new QLabel("Test pattern "
            "<a href=\"https://colorcet.com/testimage/index.html\">"
            "designed by P. Kovesi</a>:");
    test_label->setWordWrap(true);
    test_label->setOpenExternalLinks(true);
    layout->addWidget(test_label, 3, 0, 1, 2);
    _test_widget = new ColorMapTestWidget();
    layout->addWidget(_test_widget, 4, 0, 1, 2);

#ifdef Q_OS_WASM
    layout->addItem(new QSpacerItem(0, 0), 4, 3);
    layout->setColumnStretch(3, 1);
    layout->addItem(new QSpacerItem(0, 0), 5, 0, 1, 3);
    layout->setRowStretch(5, 1);
#else
    layout->setColumnStretch(0, 1);
    layout->setRowStretch(0, 1);
#endif
    widget->setLayout(layout);
    setCentralWidget(widget);

    QMenu* file_menu = menuBar()->addMenu("&File");
    QAction* file_export_png_act = new QAction("&Export as PNG...", this);
    file_export_png_act->setShortcut(QKeySequence::Save);
    connect(file_export_png_act, SIGNAL(triggered()), this, SLOT(file_export_png()));
    file_menu->addAction(file_export_png_act);
    QAction* file_export_ppm_act = new QAction("Export as PPM...", this);
    connect(file_export_ppm_act, SIGNAL(triggered()), this, SLOT(file_export_ppm()));
    file_menu->addAction(file_export_ppm_act);
    QAction* file_export_csv_act = new QAction("Export as CSV...", this);
    connect(file_export_csv_act, SIGNAL(triggered()), this, SLOT(file_export_csv()));
    file_menu->addAction(file_export_csv_act);
    QAction* file_export_hex_act = new QAction("Export as HEX...", this);
    connect(file_export_hex_act, SIGNAL(triggered()), this, SLOT(file_export_hex()));
    file_menu->addAction(file_export_hex_act);
    QAction* quit_act = new QAction("&Quit...", this);
    quit_act->setShortcut(QKeySequence::Quit);
    connect(quit_act, SIGNAL(triggered()), this, SLOT(close()));
    file_menu->addAction(quit_act);

    QMenu* edit_menu = menuBar()->addMenu("&Edit");
    QAction* edit_reset_act = new QAction("&Reset", this);
    connect(edit_reset_act, SIGNAL(triggered()), this, SLOT(edit_reset()));
    edit_menu->addAction(edit_reset_act);

    QAction* edit_copy_png_act = new QAction("&Copy as PNG", this);
    connect(edit_copy_png_act, SIGNAL(triggered()), this, SLOT(edit_copy_png()));
    edit_menu->addAction(edit_copy_png_act);
    QAction* edit_copy_ppm_act = new QAction("Copy as PPM", this);
    connect(edit_copy_ppm_act, SIGNAL(triggered()), this, SLOT(edit_copy_ppm()));
    edit_menu->addAction(edit_copy_ppm_act);
    QAction* edit_copy_csv_act = new QAction("Copy as CSV", this);
    connect(edit_copy_csv_act, SIGNAL(triggered()), this, SLOT(edit_copy_csv()));
    edit_menu->addAction(edit_copy_csv_act);
    QAction* edit_copy_hex_act = new QAction("Copy as HEX", this);
    connect(edit_copy_hex_act, SIGNAL(triggered()), this, SLOT(edit_copy_hex()));
    edit_menu->addAction(edit_copy_hex_act);
#ifdef Q_OS_WASM
    edit_copy_png_act->setEnabled(false);
    edit_copy_csv_act->setShortcut(QKeySequence::Copy);
#else
    edit_copy_png_act->setShortcut(QKeySequence::Copy);
#endif

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

ColorMapWidget* GUI::currentWidget()
{
    QTabWidget* tw = reinterpret_cast<QTabWidget*>(_category_widget->currentWidget());
    return reinterpret_cast<ColorMapWidget*>(tw->currentWidget());
}

void GUI::update()
{
    _reference_label->setText(currentWidget()->reference());
    int clipped;
    QVector<unsigned char> colormap = currentWidget()->colorMap(&clipped);
    _clipped_label->setText(QString("Colors clipped: %1").arg(clipped));
    _colormap_label->setPixmap(QPixmap::fromImage(currentWidget()->colorMapImage(colormap, 32, _colormap_label->height())));
    _test_widget->update(colormap);
}

void GUI::file_export_png()
{
    QVector<unsigned char> colormap = currentWidget()->colorMap();
    QImage image = currentWidget()->colorMapImage(colormap, 0, 1);
    QByteArray exportData;
    QBuffer buffer(&exportData);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    QFileDialog::saveFileContent(exportData, "colormap.png");
}

void GUI::file_export_ppm()
{
    QVector<unsigned char> colormap = currentWidget()->colorMap();
    QByteArray exportData = QByteArray::fromStdString(ColorMap::ToPPM(colormap.size() / 3, colormap.constData()));
    QFileDialog::saveFileContent(exportData, "colormap.ppm");
}

void GUI::file_export_csv()
{
    QVector<unsigned char> colormap = currentWidget()->colorMap();
    QByteArray exportData = QByteArray::fromStdString(ColorMap::ToCSV(colormap.size() / 3, colormap.constData()));
    QFileDialog::saveFileContent(exportData, "colormap.csv");
}

void GUI::file_export_hex()
{
    QVector<unsigned char> colormap = currentWidget()->colorMap();
    QByteArray exportData = QByteArray::fromStdString(ColorMap::ToHEX(colormap.size() / 3, colormap.constData()));
    QFileDialog::saveFileContent(exportData, "colormap.txt");
}

void GUI::edit_reset()
{
    currentWidget()->reset();
}

void GUI::edit_copy_png()
{
    QVector<unsigned char> colormap = currentWidget()->colorMap();
    QApplication::clipboard()->setImage(currentWidget()->colorMapImage(colormap, 0, 1));
}

void GUI::edit_copy_ppm()
{
    QVector<unsigned char> colormap = currentWidget()->colorMap();
    QApplication::clipboard()->setText(ColorMap::ToPPM(colormap.size() / 3, colormap.constData()).c_str());
}

void GUI::edit_copy_csv()
{
    QVector<unsigned char> colormap = currentWidget()->colorMap();
    QApplication::clipboard()->setText(ColorMap::ToCSV(colormap.size() / 3, colormap.constData()).c_str());
}

void GUI::edit_copy_hex()
{
    QVector<unsigned char> colormap = currentWidget()->colorMap();
    QApplication::clipboard()->setText(ColorMap::ToHEX(colormap.size() / 3, colormap.constData()).c_str());
}

void GUI::help_about()
{
    QMessageBox::about(this, "About",
                "<p>gencolormap version 2.6<br>"
                "   <a href=\"https://marlam.de/gencolormap\">https://marlam.de/gencolormap</a></p>"
                "<p>Copyright (C) 2025<br>"
                "   <a href=\"https://www.cg.informatik.uni-siegen.de/\">"
                "   Computer Graphics Group, University of Siegen</a> and "
                "   <a href=\"https://marlam.de/\">Martin Lambers</a>.<br>"
                "   This is free software under the terms of the "
                    "<a href=\"https://www.debian.org/legal/licenses/mit\">MIT/Expat License</a>. "
                "   There is NO WARRANTY, to the extent permitted by law."
                "</p>");
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    GUI gui;
    gui.show();
    return app.exec();
}
