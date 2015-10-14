# gencolormap

This tool generates Brewer-like color maps for visualization using intuitive parameters.
It implements the paper M. Wijffelaars, R. Vliegen, J.J. van Wijk, E.-J. van
der Linden. Generating color palettes using intuitive parameters. In Computer
Graphics Forum, vol. 27, no. 3, pp. 743-750, 2008.

The color map generation itself is contained in just two C++ files
(`colormap.hpp` and `colormap.cpp`) and requires no additional libraries.

Two frontends are included: a command line tool (for scripting) and a Qt-based
GUI (for interactive use).

![GUI screen shot](https://raw.githubusercontent.com/marlam/gencolormap/master/screenshot.png)
