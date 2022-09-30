#pragma once
#include <vector>
#include "graphics/ofImage.h"
namespace ofxImageToPalette
{
	/**
	 * @brief Creates an array of colors from an ofImage using k means clustering
	 * @param image ofImage
	 * @param k How many colors to return.
	 * @param epoch How many times clustering will be run.
	 * @param imageResize Image is resized before clustering for performance, default size = 40.
	 * @param useLabDistance If set to false euclidean distance will be used for clustering
	 * @return std::vector<ofFloatColor>
	*/
	std::vector<ofFloatColor> getPalette(ofImage image, int k, int epoch, int imageResize = 40, bool useLabDistance = true);

	/**
	 * @brief Save palettes to data folder as xml
	 * @param palettes 
	*/
	void savePalettes(std::vector<std::vector<ofFloatColor>> palettes);

	/**
	 * @brief Load palettes from data folder
	 * @return std::vector<std::vector<ofFloatColor>>
	*/
	std::vector<std::vector<ofFloatColor>> loadPalettes();
}