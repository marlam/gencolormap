HEADERS = colormap.hpp colormapwidgets.hpp export.hpp gui.hpp testwidget.hpp

SOURCES = colormap.cpp colormapwidgets.cpp export.cpp gui.cpp testwidget.cpp

RC_FILE = appicon.rc

resources.files = res/gencolormap-logo-512.png
resources.prefix = /

RESOURCES = resources

CONFIG += release

QT += widgets
