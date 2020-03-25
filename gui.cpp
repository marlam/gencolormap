/*
 * Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020
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
#include <QRadioButton>
#include <QButtonGroup>

#include "colormapwidgets.hpp"
#include "testwidget.hpp"
#include "export.hpp"


GUI::GUI()
{
    setWindowTitle("Generate Color Map");
    setWindowIcon(QIcon(":cg-logo.png"));

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

    int firstRow = 0;
#ifdef Q_OS_WASM
    QLabel *copy_hint_label = new QLabel("<b>Web Demo:</b> "
            "<i>Press CTRL+C to copy the color map in a text-based format, "
            "then save it to a file using a text editor. "
            "Other export options are only available in the native version. "
            "This is a limitation of Qt for WebAssembly.</i>");
    copy_hint_label->setWordWrap(true);
    layout->addWidget(copy_hint_label, 0, 0, 1, 2);
    firstRow = 1;
#endif

    QButtonGroup* export_format_group = new QButtonGroup;
    _export_format_png_button = new QRadioButton("PNG");
    _export_format_ppm_button = new QRadioButton("PPM");
    _export_format_csv_button = new QRadioButton("CSV");
    _export_format_json_button = new QRadioButton("JSON");
#ifdef Q_OS_WASM
    _export_format_png_button->setEnabled(false);
    _export_format_ppm_button->setChecked(true);
#else
    _export_format_png_button->setChecked(true);
#endif
    export_format_group->addButton(_export_format_png_button);
    export_format_group->addButton(_export_format_ppm_button);
    export_format_group->addButton(_export_format_csv_button);
    export_format_group->addButton(_export_format_json_button);
    QGridLayout* export_format_layout = new QGridLayout;
    QLabel* export_format_label = new QLabel("Export/Copy format: ");
    export_format_layout->addWidget(export_format_label, 0, 0);
    export_format_layout->addWidget(_export_format_png_button, 0, 1);
    export_format_layout->addWidget(_export_format_ppm_button, 0, 2);
    export_format_layout->addWidget(_export_format_csv_button, 0, 3);
    export_format_layout->addWidget(_export_format_json_button, 0, 4);
    export_format_layout->addItem(new QSpacerItem(0, 0), 0, 5);
    export_format_layout->setColumnStretch(5, 1);
    layout->addLayout(export_format_layout, firstRow++, 0, 1, 2);

    _category_widget = new QTabWidget();
    _category_seq_widget = new QTabWidget();
    _category_seq_widget->addTab(_brewerseq_widget, "Brewer-like");
    _category_seq_widget->addTab(_cubehelix_widget, "CubeHelix");
    //_category_seq_widget->addTab(_mcnames_widget, "McNames");
    _category_seq_widget->addTab(_puseq_lightness_widget, "PU Lightness");
    _category_seq_widget->addTab(_puseq_saturation_widget, "PU Saturation");
    _category_seq_widget->addTab(_puseq_rainbow_widget, "PU Rainbow");
    _category_seq_widget->addTab(_puseq_blackbody_widget, "PU Black Body");
    _category_seq_widget->addTab(_puseq_multihue_widget, "PU Multi Hue");
    connect(_category_seq_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    _category_widget->addTab(_category_seq_widget, "Sequential");
    _category_div_widget = new QTabWidget();
    _category_div_widget->addTab(_brewerdiv_widget, "Brewer-like");
    _category_div_widget->addTab(_moreland_widget, "Moreland");
    _category_div_widget->addTab(_pudiv_lightness_widget, "PU Lightness");
    _category_div_widget->addTab(_pudiv_saturation_widget, "PU Saturation");
    connect(_category_div_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    _category_widget->addTab(_category_div_widget, "Diverging");
    _category_qual_widget = new QTabWidget();
    _category_qual_widget->addTab(_brewerqual_widget, "Brewer-like");
    _category_qual_widget->addTab(_puqual_hue_widget, "PU Hue");
    connect(_category_qual_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    _category_widget->addTab(_category_qual_widget, "Qualitative");
    connect(_category_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    layout->addWidget(_category_widget, firstRow, 0);
    _reference_label = new QLabel(_brewerseq_widget->reference());
    _reference_label->setWordWrap(true);
    _reference_label->setOpenExternalLinks(true);
    layout->addWidget(_reference_label, firstRow + 1, 0);
    layout->addItem(new QSpacerItem(0, 0), firstRow + 2, 0);
    _clipped_label = new QLabel("");
    layout->addWidget(_clipped_label, firstRow + 3, 0);

    _colormap_label = new QLabel();
    _colormap_label->setScaledContents(true);
    layout->addWidget(_colormap_label, firstRow, 1, 4, 1);

    QLabel* test_label = new QLabel("Test pattern "
            "<a href=\"http://peterkovesi.com/projects/colourmaps/colourmaptestimage.html\">"
            "designed by P. Kovesi</a>:");
    test_label->setWordWrap(true);
    test_label->setOpenExternalLinks(true);
    layout->addWidget(test_label, firstRow + 4, 0, 1, 2);
    _test_widget = new ColorMapTestWidget();
    layout->addWidget(_test_widget, firstRow + 5, 0, 1, 2);

    QLabel* hint_label = new QLabel("Tip: Export as JSON and evaluate at <a href=\"https://colormeasures.org/\">colormeasures.org</a>.");
    hint_label->setOpenExternalLinks(true);
    layout->addWidget(hint_label, firstRow + 6, 0, 1, 2);

    layout->setColumnStretch(0, 1);
    layout->setRowStretch(firstRow + 2, 1);
    widget->setLayout(layout);
    setCentralWidget(widget);

    QMenu* file_menu = menuBar()->addMenu("&File");
    QAction* file_export_act = new QAction("&Export...", this);
    file_export_act->setShortcut(QKeySequence::Save);
    connect(file_export_act, SIGNAL(triggered()), this, SLOT(file_export()));
    file_menu->addAction(file_export_act);
#ifdef Q_OS_WASM
    file_export_act->setEnabled(false);
#endif
    QAction* quit_act = new QAction("&Quit...", this);
    quit_act->setShortcut(QKeySequence::Quit);
    connect(quit_act, SIGNAL(triggered()), this, SLOT(close()));
    file_menu->addAction(quit_act);

    QMenu* edit_menu = menuBar()->addMenu("&Edit");
    QAction* edit_reset_act = new QAction("&Reset", this);
    connect(edit_reset_act, SIGNAL(triggered()), this, SLOT(edit_reset()));
    edit_menu->addAction(edit_reset_act);
    QAction* edit_copy_act = new QAction("&Copy", this);
    edit_copy_act->setShortcut(QKeySequence::Copy);
    connect(edit_copy_act, SIGNAL(triggered()), this, SLOT(edit_copy()));
    edit_menu->addAction(edit_copy_act);

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

void GUI::file_export()
{
    QString name = QFileDialog::getSaveFileName();
    if (!name.isEmpty()) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QVector<unsigned char> colormap = currentWidget()->colorMap();
        if (_export_format_png_button->isChecked()) {
            currentWidget()->colorMapImage(colormap, 0, 1).save(name, "png");
        } else {
            QFile file(name);
            if (file.open(QIODevice::WriteOnly)) {
                QTextStream stream(&file);
                std::string exportedColormap;
                if (_export_format_ppm_button->isChecked()) {
                    exportedColormap = ColorMap::ToPPM(colormap.size() / 3, colormap.constData());
                } else if (_export_format_csv_button->isChecked()) {
                    exportedColormap = ColorMap::ToCSV(colormap.size() / 3, colormap.constData());
                } else if (_export_format_json_button->isChecked()) {
                    exportedColormap = ColorMap::ToJSON(colormap.size() / 3, colormap.constData());
                }
                stream << exportedColormap.c_str();
            }
        }
        QApplication::restoreOverrideCursor();
    }
}

void GUI::edit_reset()
{
    currentWidget()->reset();
}

void GUI::edit_copy()
{
    QVector<unsigned char> colormap = currentWidget()->colorMap();
    if (_export_format_png_button->isChecked()) {
        QApplication::clipboard()->setImage(currentWidget()->colorMapImage(colormap, 0, 1));
    } else {
        std::string exportedColormap;
        if (_export_format_ppm_button->isChecked()) {
            exportedColormap = ColorMap::ToPPM(colormap.size() / 3, colormap.constData());
        } else if (_export_format_csv_button->isChecked()) {
            exportedColormap = ColorMap::ToCSV(colormap.size() / 3, colormap.constData());
        } else if (_export_format_json_button->isChecked()) {
            exportedColormap = ColorMap::ToJSON(colormap.size() / 3, colormap.constData());
        }
        QApplication::clipboard()->setText(exportedColormap.c_str());
    }
}

void GUI::help_about()
{
    QMessageBox::about(this, "About",
                "<p>gencolormap version 2.0<br>"
                "   <a href=\"https://marlam.de/gencolormap\">https://marlam.de/gencolormap</a></p>"
                "<p>Copyright (C) 2020<br>"
                "   <a href=\"http://www.cg.informatik.uni-siegen.de/\">"
                "   Computer Graphics Group, University of Siegen</a>.<br>"
                "   Written by <a href=\"https://marlam.de/\">Martin Lambers</a>.<br>"
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
