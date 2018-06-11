# gencolormap

These tools generate color maps for visualization.
A variety of methods for sequential, diverging, and qualitative maps is available.

The color map generation is contained in just two C++ files (`colormap.hpp` and
`colormap.cpp`) and requires no additional libraries.

Two frontends are included: a GUI for interactive use and a command line tool
for scripting.

The following papers are implemented:
* M. Wijffelaars, R. Vliegen, J.J. van Wijk, E.-J. van der Linden.
  [Generating color palettes using intuitive parameters](http://dx.doi.org/10.1111/j.1467-8659.2008.01203.x).
  Computer Graphics Forum 27(3), May 2008.
* D. A. Green.
  [A colour scheme for the display of astronomical intensity](http://www.mrao.cam.ac.uk/~dag/CUBEHELIX/).
  Bulletin of the Astronomical Society of India 39(2), June 2011.
* K. Moreland.
  [Diverging Color Maps for Scientific Visualization](http://dx.doi.org/10.1007/978-3-642-10520-3_9).
  Proc. Int. Symp. Visual Computing, December 2009.
* J. McNames.
  [An Effective Color Scale for Simultaneous Color and Gray-Scale Publications](http://dx.doi.org/10.1109/MSP.2006.1593340).
  IEEE Signal Processing Magazine 23(1), January 2006.

![GUI screen shot](https://gitlab.marlam.de/marlam/gencolormap/raw/master/screenshot.png)
