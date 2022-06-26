# Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022
# Computer Graphics Group, University of Siegen
# Written by Martin Lambers <martin.lambers@uni-siegen.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

cmake_minimum_required(VERSION 3.10)
set(CMAKE_AUTOMOC ON)

project(gencolormap)

set(CMAKE_CXX_STANDARD 11)
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

find_package(Qt6 6.2.0 COMPONENTS Widgets QUIET)

add_executable(gencolormap cmdline.cpp colormap.hpp colormap.cpp export.hpp export.cpp)
install(TARGETS gencolormap RUNTIME DESTINATION bin)

if(Qt6Widgets_FOUND)
        qt6_add_resources(GUI_RESOURCES gui.qrc)
	add_executable(gencolormap-gui gui.cpp
		colormapwidgets.hpp colormapwidgets.cpp
                testwidget.hpp testwidget.cpp
		export.hpp export.cpp
		colormap.hpp colormap.cpp ${GUI_RESOURCES})
	target_link_libraries(gencolormap-gui Qt6::Widgets)
	install(TARGETS gencolormap-gui RUNTIME DESTINATION bin)
endif()
