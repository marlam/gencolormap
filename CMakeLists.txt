# Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022
# Computer Graphics Group, University of Siegen
# Written by Martin Lambers <martin.lambers@uni-siegen.de>
# Copyright (C) 2022, 2023, 2024 Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

cmake_minimum_required(VERSION 3.20)
set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)
set(CMAKE_AUTOMOC ON)

project(gencolormap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(Qt6 6.2.0 COMPONENTS Widgets QUIET)

add_executable(gencolormap cmdline.cpp colormap.hpp colormap.cpp export.hpp export.cpp)
install(TARGETS gencolormap RUNTIME DESTINATION bin)

if(Qt6Widgets_FOUND)
	add_executable(gencolormap-gui
		gui.cpp
		colormapwidgets.hpp colormapwidgets.cpp
		testwidget.hpp testwidget.cpp
		export.hpp export.cpp
		colormap.hpp colormap.cpp
		appicon.rc)
	qt6_add_resources(gencolormap-gui "misc" PREFIX "/" FILES aux/gencolormap-logo-512.png)
	set_target_properties(gencolormap-gui PROPERTIES WIN32_EXECUTABLE TRUE)
	target_link_libraries(gencolormap-gui Qt6::Widgets)
	install(TARGETS gencolormap-gui RUNTIME DESTINATION bin)
	# Add auxiliary files for Linux-ish systems
	if(UNIX)
		install(FILES "aux/gencolormap-logo-16.png"  RENAME "de.marlam.gencolormap.png" DESTINATION share/icons/hicolor/16x16/apps)
		install(FILES "aux/gencolormap-logo-22.png"  RENAME "de.marlam.gencolormap.png" DESTINATION share/icons/hicolor/22x22/apps)
		install(FILES "aux/gencolormap-logo-32.png"  RENAME "de.marlam.gencolormap.png" DESTINATION share/icons/hicolor/32x32/apps)
		install(FILES "aux/gencolormap-logo-48.png"  RENAME "de.marlam.gencolormap.png" DESTINATION share/icons/hicolor/48x48/apps)
		install(FILES "aux/gencolormap-logo-64.png"  RENAME "de.marlam.gencolormap.png" DESTINATION share/icons/hicolor/64x64/apps)
		install(FILES "aux/gencolormap-logo-128.png" RENAME "de.marlam.gencolormap.png" DESTINATION share/icons/hicolor/128x128/apps)
		install(FILES "aux/gencolormap-logo-256.png" RENAME "de.marlam.gencolormap.png" DESTINATION share/icons/hicolor/256x256/apps)
		install(FILES "aux/gencolormap-logo-512.png" RENAME "de.marlam.gencolormap.png" DESTINATION share/icons/hicolor/512x512/apps)
		install(FILES "aux/gencolormap-logo.svg"     RENAME "de.marlam.gencolormap.svg" DESTINATION share/icons/hicolor/scalable/apps)
		install(FILES "aux/de.marlam.gencolormap.desktop" DESTINATION share/applications)
		install(FILES "aux/de.marlam.gencolormap.metainfo.xml" DESTINATION share/metainfo)
	endif()
endif()
