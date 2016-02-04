# Copyright (C) 2015, 2016
# Computer Graphics Group, University of Siegen
# Written by Martin Lambers <martin.lambers@uni-siegen.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

cmake_minimum_required(VERSION 2.8)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
endif()

find_package(Qt5Widgets QUIET)

add_executable(gencolormap cmdline.cpp colormap.hpp colormap.cpp)
install(TARGETS gencolormap RUNTIME DESTINATION bin)

if(Qt5Widgets_FOUND)
        qt5_add_resources(GUI_RESOURCES gui.qrc)
	add_executable(gencolormap-gui gui.cpp
		colormapwidgets.hpp colormapwidgets.cpp
		colormap.hpp colormap.cpp ${GUI_RESOURCES})
	target_link_libraries(gencolormap-gui Qt5::Widgets)
	install(TARGETS gencolormap-gui RUNTIME DESTINATION bin)
endif()
