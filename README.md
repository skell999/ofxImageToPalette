ofxImageToPalette
================

OpenFrameworks addon for extracting a palette from an image.
ofxImageToPalette uses k mean clustering to group similar pixels. Pixel distance measured in L\*A\*B\* color space.
Converting images to palettes is not deterministic. Every time ```getPalette()``` is run on an image it will produce a differant output.

--------
![Van Gogh Starry Night Palette](https://github.com/skell999/ofxImageToPalette/blob/main/images/vangogh.jpeg "Van Gogh Starry Night")

Usage
--------

Basic
```c++
ofImage image;
image.load("vangogh.jpeg");
std::vector<ofFloatColor> palette = ofxImageToPalette::getPalette(image,5,3);
```
Save palettes to data folder
```c++
std::vector<std::vector<ofFloatColor>> palettes;
ofImage image;
image.load("vangogh.jpeg");
palettes.push_back(ofxImageToPalette::getPalette(image,5,3));
ofxImageToPalette::savePalettes(palettes);
```
Load palettes from data folder
```c++
std::vector<std::vector<ofFloatColor>> palettes = loadPalettes();
```
Parameters
-------------
* ```ofimage image```
* ```int k``` Controls how many colors will be in the final output.
* ```int epoch``` Controls how many k means clustering iterations will be run, higher values will return colors that are more washed out.
* ```int imageResize``` Image will be resized before being processed for performance.
* ```bool useLabDistance``` If set to false color will measured with euclidean distance.

Limitations
-------------
* Three channel images only.
* Can only return ofFloatColor palettes.

Further work
-------------
* Option to create a deterministic output.
* Add functions to change a palettes brightness, contrast, etc.
* Experiment with using ofColor.getLightness() to filter out similar colors.
* Experiment with using L\*A\*B\* distance to pick colors that are very unique.
* Add a function that will draw an imgui palette editor.
