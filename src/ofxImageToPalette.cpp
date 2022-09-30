#include "ofxImageToPalette.h"
#include "ofMath.h"
#include "math/ofVec3f.h"
#include "ofXml.h"
namespace ofxImageToPalette
{

	//ported from http://cookbooks.adobe.com/post_Useful_color_equations__RGB_to_LAB_converter-14227.html
	struct Color {
		float R, G, B, X, Y, Z, L, a, b;
	};

	const float REF_X = 95.047f; // Observer= 2°, Illuminant= D65
	const float REF_Y = 100.000;
	const float REF_Z = 108.883;

	Color rgb2xyz(float r, float g, float b) {
		//float r = R / 255.0;
		//float g = G / 255.0;
		//float b = B / 255.0;

		if (r > 0.04045) { r = pow((r + 0.055) / 1.055, 2.4); }
		else { r = r / 12.92; }
		if (g > 0.04045) { g = pow((g + 0.055) / 1.055, 2.4); }
		else { g = g / 12.92; }
		if (b > 0.04045) { b = pow((b + 0.055) / 1.055, 2.4); }
		else { b = b / 12.92; }

		r = r * 100;
		g = g * 100;
		b = b * 100;
		//Observer. = 2°, Illuminant = D65
		Color xyz;
		xyz.X = r * 0.4124 + g * 0.3576 + b * 0.1805;
		xyz.Y = r * 0.2126 + g * 0.7152 + b * 0.0722;
		xyz.Z = r * 0.0193 + g * 0.1192 + b * 0.9505;
		return xyz;
	}
	Color xyz2lab(float X, float Y, float Z) {
		float x = X / REF_X;
		float y = Y / REF_X;
		float z = Z / REF_X;

		if (x > 0.008856) { x = pow(x, .3333333333f); }
		else { x = (7.787 * x) + (16 / 116.0); }
		if (y > 0.008856) { y = pow(y, .3333333333f); }
		else { y = (7.787 * y) + (16 / 116.0); }
		if (z > 0.008856) { z = pow(z, .3333333333f); }
		else { z = (7.787 * z) + (16 / 116.0); }

		Color lab;
		lab.L = (116 * y) - 16;
		lab.a = 500 * (x - y);
		lab.b = 200 * (y - z);
		return lab;
	}
	Color lab2xyz(float l, float a, float b) {
		float y = (l + 16) / 116;
		float x = a / 500 + y;
		float z = y - b / 200;

		if (pow(y, 3) > 0.008856) { y = pow(y, 3); }
		else { y = (y - 16 / 116) / 7.787; }
		if (pow(x, 3) > 0.008856) { x = pow(x, 3); }
		else { x = (x - 16 / 116) / 7.787; }
		if (pow(z, 3) > 0.008856) { z = pow(z, 3); }
		else { z = (z - 16 / 116) / 7.787; }

		Color xyz;
		xyz.X = x * REF_X;
		xyz.Y = y * REF_Y;
		xyz.Z = z * REF_Z;
		return xyz;
	}
	Color xyz2rgb(float X, float Y, float Z) {
		//X from 0 to  95.047      (Observer = 2°, Illuminant = D65)
		//Y from 0 to 100.000
		//Z from 0 to 108.883
		X = ofClamp(X, 0, 95.047);

		float x = X * .01;
		float y = Y * .01;
		float z = Z * .01;

		float r = x * 3.2406 + y * -1.5372 + z * -0.4986;
		float g = x * -0.9689 + y * 1.8758 + z * 0.0415;
		float b = x * 0.0557 + y * -0.2040 + z * 1.0570;

		if (r > 0.0031308) { r = 1.055 * pow(r, (1 / 2.4f)) - 0.055; }
		else { r = 12.92 * r; }
		if (g > 0.0031308) { g = 1.055 * pow(g, (1 / 2.4f)) - 0.055; }
		else { g = 12.92 * g; }
		if (b > 0.0031308) { b = 1.055 * pow(b, (1 / 2.4f)) - 0.055; }
		else { b = 12.92 * b; }

		Color rgb;
		rgb.R = round(r * 255);
		rgb.G = round(g * 255);
		rgb.B = round(b * 255);
		return rgb;
	}
	Color rgb2lab(float R, float G, float B) {
		Color xyz = rgb2xyz(R, G, B);
		return xyz2lab(xyz.X, xyz.Y, xyz.Z);
	}
	Color lab2rgb(int L, int a, int b) {
		Color xyz = lab2xyz(L, a, b);
		return xyz2rgb(xyz.X, xyz.Y, xyz.Z);
	}

	std::pair<std::vector<ofVec3f>, std::vector<int>> kMeansClustering(std::vector<ofVec3f> data, int k, int epoch, bool useLabDistance)
	{
		std::vector<int> centroidIDs;
		std::vector<float> distToCentroids;
		std::vector<ofVec3f> centroids;

		// https://github.com/berendeanicolae/ColorSpace

		centroidIDs.resize(data.size(), -1);
		distToCentroids.resize(data.size(), std::numeric_limits<float>::max());
		centroids.resize(k, { 0,0,0 });

		// Init centroids from data
		for (auto& c : centroids)
		{
			size_t randIndex = (size_t)std::floor(ofRandom(data.size()));
			assert(randIndex < data.size());
			c = data[randIndex];
		}

		for (size_t i = 0; i < epoch; i++)
		{
			// Assign new distances
			for (size_t j = 0; j < centroids.size(); j++)
			{
				for (size_t d = 0; d < data.size(); d++)
				{
					float distance = 0;
					if (useLabDistance) {
						auto blah = rgb2lab(255, 0, 0);
						auto centroidLab = rgb2lab(centroids[j].x * 1.f, centroids[j].y * 1.f, centroids[j].z * 1.f);
						auto dataLab = rgb2lab(data[d].x * 1.f, data[d].y * 1.f, data[d].z * 1.f);
						ofVec3f centroidPoint = { centroidLab.L, centroidLab.a, centroidLab.b, };
						ofVec3f dataPoint = { dataLab.L, dataLab.a, dataLab.b, };
						distance = dataPoint.squareDistance(centroidPoint);
					}
					else
					{
						distance = data[d].squareDistance(centroids[j]);
					}

					if (distance < distToCentroids[d])
					{
						distToCentroids[d] = distance;
						centroidIDs[d] = j;
					}
				}
			}

			// Recalc centroids
			for (size_t c = 0; c < centroids.size(); c++)
			{
				float nPoints = 0;
				ofVec3f sum = { 0,0,0 };
				for (size_t d = 0; d < data.size(); d++)
				{
					if (centroidIDs[d] == c)
					{
						nPoints++;
						sum += data[d];
					}
				}
				if (nPoints)
				{
					centroids[c] = sum / nPoints;
				}
			}
		}
		return { centroids,centroidIDs };
	}

	std::vector<ofVec3f> imagToVec3f(ofImage& image)
	{
		std::vector<ofVec3f> data;
		size_t numPixels = 0;
		size_t size = image.getPixels().size();
		ofImageType imageType = image.getImageType();
		switch (imageType)
		{
		case OF_IMAGE_GRAYSCALE:
			numPixels = size;
			break;
		case OF_IMAGE_COLOR:
			numPixels = size / 3;
			break;
		case OF_IMAGE_COLOR_ALPHA:
			numPixels = size / 4;
			break;
		case OF_IMAGE_UNDEFINED:
			throw std::invalid_argument("Invalid image type");
			break;
		default:
			break;
		}
		data.reserve(numPixels);

		for (size_t i = 0; i < numPixels; i += 3)
		{
			ofFloatColor color = image.getColor(i);
			ofVec3f point = { (float)color.r / 255.f,(float)color.g / 255.f,(float)color.b / 255.f };
			data.push_back(point);
		}
		return data;
	}

	std::vector<ofFloatColor> vec3fToColor(std::vector<ofVec3f> data)
	{
		std::vector<ofFloatColor> colors;
		colors.reserve(data.size());
		for (auto p : data)
		{
			colors.push_back({ p.x * 255, p.y * 255, p.z * 255 });
		}
		return colors;
	}

	std::vector<ofFloatColor> getPalette(ofImage image, int k, int epoch, int imageResize, bool useLabDistance)
	{
		std::vector<ofVec3f> centroids;
		std::vector<int> clusterCount;
		image.resize(imageResize, imageResize);
		std::tie(centroids, clusterCount) = kMeansClustering(imagToVec3f(image), k, epoch, useLabDistance);
		std::vector<ofFloatColor> colors = vec3fToColor(centroids);
		std::sort(colors.begin(), colors.end(), [](ofColor a, ofColor b) { return a.getLightness() > b.getLightness(); });
		return colors;
	}

	void savePalettes(std::vector<std::vector<ofFloatColor>> palettes)
	{
		ofXml xml;
		size_t vectorCount = 0;
		size_t colorCount = 0;
		xml.appendAttribute("palettes");
		for (auto& vec : palettes)
		{
			xml.appendChild("p" + ofToString(vectorCount)).set(ofToString(vec));
			vectorCount++;
		}
		xml.save("palettes.xml");
	}

	std::vector<std::vector<ofFloatColor>> loadPalettes()
	{
		std::vector<std::vector<ofFloatColor>> palettes;
		ofXml xml;
		if (xml.load("palettes.xml"))
		{
			for (auto& at : xml.getChildren())
			{
				std::vector<ofFloatColor> palette;
				auto split = ofSplitString(at.getValue(), ",");
				ofStringReplace(split.front(), "{", "");
				ofStringReplace(split.back(), "}", "");
				for (auto& str : split)
				{
					ofStringReplace(str, " ", "");
				}

				palette.reserve(split.size());
				for (size_t i = 0; i < split.size(); i += 4)
				{
					ofFloatColor c = { ofToFloat(split[i]), ofToFloat(split[i + 1]), ofToFloat(split[i + 2]), ofToFloat(split[i + 3]), };
					palette.push_back(c);
				}

				palettes.push_back(palette);			
			}
		}
		return palettes;
	}
}