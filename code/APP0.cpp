#include <iostream>
#include <vector>
#include <map>
#include "APP0.h"

unsigned char APP0::marker[2] = { 0xFF, 0xE0 };
map<int, string> APP0::densityUnitsMap = {
	{0, "No units; width:height pixel aspect ratio = Ydensity:Xdensity"},
	{1, "Pixels per inch (2.54 cm)"},
	{2, "Pixels per centimeter"}
};

APP0::APP0(JPEGCodec* jpegCodec): Segment(jpegCodec){

}

APP0::~APP0(){
	if (thumbnailData != NULL){
		delete[] thumbnailData;
	}
}

bool APP0::Decode(vector<unsigned char>& image, int& pos){
	if (image[pos] != marker[0] || image[pos+1] != marker[1]){
		return false;
	}
	pos = pos + 2;

	// Get Length
	length = image[pos] * 256 + image[pos+1];
	pos = pos + 2;

	// Get Identifier
	for (int i = 0 ; i < 5 ; i++, pos++){
		identifier[i] = image[pos];
	}

	// Get JFIF version
	JFIFversion = "";
	if (image[pos+1] < 10){
		JFIFversion = JFIFversion + to_string(image[pos]) + ".0" + to_string(image[pos+1]);
	} else {
		JFIFversion = JFIFversion + to_string(image[pos]) + "." + to_string(image[pos+1]);
	}
	pos = pos + 2;

	// Get Density units
	densityUnits = image[pos];
	pos = pos + 1;

	// Get Xdensity and Ydensity
	xDensity = image[pos] * 256 + image[pos+1];
	yDensity = image[pos+2] * 256 + image[pos+3];
	pos = pos + 4;

	// Get Xthumbnail and Ythumbnail
	xThumbnail = image[pos];
	yThumbnail = image[pos+1];
	pos = pos + 2;

	// Get Thumbnail data
	thumbnailDataCount = 3 * xThumbnail * yThumbnail;
	if (thumbnailDataCount != 0){
		if (thumbnailData != NULL){
			delete thumbnailData;
		}
		thumbnailData = new int[thumbnailDataCount];
		for (int i = 0 ; i < thumbnailDataCount ; i++, pos++){
			thumbnailData[i] = image[pos];
		}
	}

	return true;
}

void APP0::Display(){
	cout << "APP0 segment" << endl;
	cout << "Length: " << length << endl;
	cout << "Identifier: " << identifier << endl;
	cout << "JFIFversion: " << JFIFversion << endl;
	cout << "Density units: " << densityUnits << " (" << densityUnitsMap[densityUnits] << ")" << endl;
	cout << "Density: " << xDensity << " x " << yDensity << endl;
	cout << "Thumbnail: " << xThumbnail << " x " << yThumbnail << endl;
	cout << "Has thumbnail? " << (thumbnailData!=NULL ? "Yes" : "No") << endl;
}