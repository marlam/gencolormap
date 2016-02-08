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
#include <QTabWidget>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QClipboard>
#include <QTextStream>
#include <QMessageBox>

#include "colormapwidgets.hpp"


GUI::GUI()
{
    setWindowTitle("Generate Color Map");
    setWindowIcon(QIcon(":cg-logo.png"));

    _brewerseq_widget = new ColorMapBrewerSequentialWidget;
    _brewerdiv_widget = new ColorMapBrewerDivergingWidget;
    _brewerqual_widget = new ColorMapBrewerQualitativeWidget;
    _isolumseq_widget = new ColorMapIsoluminantSequentialWidget;
    _isolumdiv_widget = new ColorMapIsoluminantDivergingWidget;
    _isolumqual_widget = new ColorMapIsoluminantQualitativeWidget;
    _cubehelix_widget = new ColorMapCubeHelixWidget;
    _moreland_widget = new ColorMapMorelandWidget;
    _mcnames_widget = new ColorMapMcNamesWidget;
    connect(_brewerseq_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_brewerdiv_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_brewerqual_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_isolumseq_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_isolumdiv_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_isolumqual_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_cubehelix_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_moreland_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));
    connect(_mcnames_widget, SIGNAL(colorMapChanged()), this, SLOT(update()));

    QWidget *widget = new QWidget;
    widget->setMinimumWidth(384 * qApp->devicePixelRatio());
    QGridLayout *layout = new QGridLayout;

    _category_widget = new QTabWidget();
    _category_seq_widget = new QTabWidget();
    _category_seq_widget->addTab(_brewerseq_widget, "Brewer-like");
    _category_seq_widget->addTab(_isolumseq_widget, "Isoluminant");
    _category_seq_widget->addTab(_cubehelix_widget, "CubeHelix");
    //_category_seq_widget->addTab(_mcnames_widget, "McNames");
    connect(_category_seq_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    _category_widget->addTab(_category_seq_widget, "Sequential");
    _category_div_widget = new QTabWidget();
    _category_div_widget->addTab(_brewerdiv_widget, "Brewer-like");
    _category_div_widget->addTab(_isolumdiv_widget, "Isoluminant");
    _category_div_widget->addTab(_moreland_widget, "Moreland");
    connect(_category_div_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    _category_widget->addTab(_category_div_widget, "Diverging");
    _category_qual_widget = new QTabWidget();
    _category_qual_widget->addTab(_brewerqual_widget, "Brewer-like");
    _category_qual_widget->addTab(_isolumqual_widget, "Isoluminant");
    connect(_category_qual_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    _category_widget->addTab(_category_qual_widget, "Qualitative");
    connect(_category_widget, SIGNAL(currentChanged(int)), this, SLOT(update()));
    layout->addWidget(_category_widget, 0, 0);
    layout->addItem(new QSpacerItem(0, 0), 1, 0);
    _reference_label = new QLabel(_brewerseq_widget->reference());
    _reference_label->setWordWrap(true);
    _reference_label->setOpenExternalLinks(true);
    layout->addWidget(_reference_label, 2, 0);

    _colormap_label = new QLabel();
    _colormap_label->setScaledContents(true);
    layout->addWidget(_colormap_label, 0, 1, 3, 1);

    layout->setColumnStretch(0, 1);
    layout->setRowStretch(1, 1);
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

ColorMapWidget* GUI::currentWidget()
{
    QTabWidget* tw = reinterpret_cast<QTabWidget*>(_category_widget->currentWidget());
    return reinterpret_cast<ColorMapWidget*>(tw->currentWidget());
}

void GUI::update()
{
    _reference_label->setText(currentWidget()->reference());
    _colormap_label->setPixmap(QPixmap::fromImage(currentWidget()->colorMapImage(32, _colormap_label->height())));
}

void GUI::file_export_png()
{
    QString name = QFileDialog::getSaveFileName();
    if (!name.isEmpty()) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        currentWidget()->colorMapImage(0, 1).save(name, "png");
        QApplication::restoreOverrideCursor();
    }
}

void GUI::file_export_csv()
{
    QString name = QFileDialog::getSaveFileName();
    if (!name.isEmpty()) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QFile file(name);
        if (file.open(QIODevice::WriteOnly)) {
            QVector<QColor> colormap = currentWidget()->colorMap();
            QTextStream stream(&file);
            for (int i = 0; i < colormap.size(); i++) {
                stream << colormap[i].red()   << ", "
                       << colormap[i].green() << ", "
                       << colormap[i].blue()  << endl;
            }
        }
        QApplication::restoreOverrideCursor();
    }
}

void GUI::edit_reset()
{
    currentWidget()->reset();
}

void GUI::edit_copy_as_img()
{
    QApplication::clipboard()->setImage(currentWidget()->colorMapImage(0, 1));
}

void GUI::edit_copy_as_txt()
{
    QVector<QColor> colormap = currentWidget()->colorMap();
    QString string;
    QTextStream stream(&string);
    for (int i = 0; i < colormap.size() / 3; i++) {
        stream << colormap[i].red()   << ", "
               << colormap[i].green() << ", "
               << colormap[i].blue()  << endl;
    }
    QApplication::clipboard()->setText(string);
}

void GUI::help_about()
{
    QMessageBox::about(this, "About",
                "<p>gencolormap version 0.3</p>"
                "<p>Copyright (C) 2016<br>"
                "   <a href=\"http://www.cg.informatik.uni-siegen.de/\">"
                "   Computer Graphics Group, University of Siegen</a>.<br>"
                "   Written by <a href=\"http://www.cg.informatik.uni-siegen.de/lambers-martin\">Martin Lambers</a>.<br>"
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
