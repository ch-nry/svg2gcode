﻿# svg-line_2_gcode 
allow to convert svg file exported by librecad (MakerCAM SVG)
as a Gcode file compatible with most CNC
	
# compilation
make

# usage
svg-line_2_gcode file.svg
->output : file.gcode

This is a very crude tool, it works only with svg file generated by librecad 
as MakerCAM SVG. A modification in the exportation syntax will probably break
the conversion. It was design around librecad version 2.1.3

A generic SVG file will not work with this tool

A SVG example file is provided, as well as the libreCAD dxf file used to generate this SVG.

# file preparation
- create your design on one or multiple layer (construction layer) that will be excluded on the final export
- create a layer with the name : label depth pass
	replace "label" is a single word text like "exteral_cut"
	replace "depth" with a float number representing the depth of the cut, like "12.3"
	replace "pass" with an int number representing the number of pass
- draw with 2 points line on this layer the cut you want
	the aim is to use the construction layer as a support to quickly draw the movement of the tool
	the tool will follow the order of the segment creation
- replicate this operations (layer creation and segments drawing) for all tool movement you want
- display all and only the layer you want to export and convert as a Gcode
- export as a MakerCAM SVG
	check the option "Convert blocks to safe inline SVG content" (it should already be checked)
- use this tool as a command line to convert the svg file to a gcode
- use a text editor and / or online visualiser to check the file, and eventually edit it. 
	lot's of comment are added in the file in order to allow an easy manual edition

# customisation
Without proper integration in libreCAD, it's not easy to customise the Gcode.
To change the moving speed, cut speed, clearance level, you need to change the 
define in the source code and compile it again

